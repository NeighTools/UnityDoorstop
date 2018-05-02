/*
 * main.cpp -- The main "entry point" and the main logic of the DLL.
 *
 * Here, we define and initialize struct Main that contains the main code of this DLL.
 * 
 * The main procedure goes as follows:
 * 1. The loader checks that PatchLoader.dll and mono.dll exist
 * 2. mono.dll is loaded into memory and some of its functions are looked up
 * 3. mono_jit_init_version is hooked with the help of MinHook
 * 
 * Then, the loader waits until Unity creates its root domain for mono (which is done with mono_jit_init_version).
 * 
 * Inside mono_jit_init_version hook:
 * 1. Call the original mono_jit_init_version to get the Unity root domain
 * 2. Load PatchLoader.dll into the root domain
 * 3. Find and invoke PatchLoader.Loader.Run()
 * 
 * Rest of the work is done on the managed side.
 *
 */

#include <windows.h>
#include <string>
#include <atlstr.h>
#include <ctime>
#include <Psapi.h>
#include <Shlwapi.h>
#include "../lib/minhook/include/MinHook.h"

#include "logger.h"

#define FINI_WIDE_SUPPORT

#define VERSION "1.1"
#define CONFIG_NAME L"upp_config.ini"
#define DEFAULT_LOADER_PATH L"UnityPrePatcher"

// A helper for cleaner error logging
#define ASSERT(test, message, ...)				  \
	if(!(test))									  \
	{											  \
		LOG(Logger::getTimeString() << message);  \
		return __VA_ARGS__;						  \
	}

#include "mono.h"
using namespace Mono;

namespace MonoLoader
{
	struct Configuration
	{
		bool enabled = true;
		CStringW uppPath = DEFAULT_LOADER_PATH;
		CStringW loaderDllPath;

		void init()
		{
			loadConfig();
			loaderDllPath = uppPath + L"\\bin\\PatchLoader.dll";
		}

		void loadConfig()
		{
			wchar_t curPathProcess[MAX_PATH];
			GetModuleFileNameW(nullptr, curPathProcess, sizeof(curPathProcess));

			wchar_t curPath[_MAX_DIR];
			wchar_t drive[_MAX_DRIVE];
			_wsplitpath_s(curPathProcess, drive, sizeof(drive), curPath, sizeof(curPath), nullptr, 0, nullptr, 0);

			CStringW curPathStr = drive;
			curPathStr += curPath;
			curPathStr += CONFIG_NAME;

			wchar_t enabledString[256] = L"true";
			GetPrivateProfileStringW(L"UnityPrePatcher", L"enabled", L"true", enabledString, sizeof(enabledString), curPathStr);

			if (_wcsnicmp(enabledString, L"true", 4) == 0)
				enabled = true;
			if (_wcsnicmp(enabledString, L"false", 5) == 0)
				enabled = false;

			wchar_t uppPathStr[MAX_PATH] = DEFAULT_LOADER_PATH;
			GetPrivateProfileStringW(L"UnityPrePatcher", L"loaderPath", DEFAULT_LOADER_PATH, uppPathStr, sizeof(uppPathStr), curPathStr);

			uppPath = uppPathStr;
		}
	};

	static Configuration configuration;

	// The hook for mono_jit_init_version
	// We use this since it will always be called once to initialize Mono's JIT
	void* ownMonoJitInitVersion(const char* root_domain_name, const char* runtime_version)
	{
		// Call the original mono_jit_init_version to initialize the Unity Root Domain
		const auto domain = mono_jit_init_version_original(root_domain_name, runtime_version);

		const CStringA dllPath(configuration.loaderDllPath);

		// Load our custom assembly into the domain
		const auto assembly = mono_domain_assembly_open(domain, dllPath);
		ASSERT(assembly != nullptr, " Failed to load PatchLoader.dll as a CLR assembly!", domain);

		// Get assembly's image that contains CIL code
		const auto image = mono_assembly_get_image(assembly);
		ASSERT(image != nullptr, " Failed to get assembly image for PatchLoader.dll!", domain);

		// Find our Loader class from the assembly
		const auto classDef = mono_class_from_name(image, "PatchLoader", "Loader");
		ASSERT(classDef != nullptr, " Failed to load class PatchLoader.Loader!", domain);

		// Create a method descriptor that is used to find the Run() method
		const auto descriptor = mono_method_desc_new("Loader:Run", FALSE);

		// Find Run() method from Loader class
		const auto method = mono_method_desc_search_in_class(descriptor, classDef);
		ASSERT(method != nullptr, " Failed to locate Loader.Run() method!", domain);

		// Invoke Run() with no parameters
		mono_runtime_invoke(method, nullptr, nullptr, nullptr);

		return domain;
	}

	struct Main
	{
		Main()
		{
			configuration.init();

			// If the loader is disabled, don't inject anything.
			if (!configuration.enabled)
				return;

			ASSERT(PathFileExistsW(configuration.uppPath), " No UnityPrePatcher folder found! Aborting...");
			ASSERT(PathFileExistsW(configuration.loaderDllPath), " No PatchLoader.dll found! Aborting...");

			const auto monoDllPath = getMonoPath();

			ASSERT(PathFileExistsW(monoDllPath), " No mono.dll found from" << monoDllPath << "! Aborting...");

			// Preload mono into memory so we can start hooking it
			const auto monoLib = LoadLibrary(monoDllPath);

			ASSERT(monoLib != nullptr, " Failed to load mono.dll! Aborting...");

			loadMonoFunctions(monoLib);

			// Initialize MinHook
			// TODO: Error handling
			auto status = MH_Initialize();

			ASSERT(status == MH_OK, " Failed to initialize MinHook! Error code: " << status);

			// Add a detour to mono_jit_init_version
			status = MH_CreateHook(mono_jit_init_version, &ownMonoJitInitVersion,
			                                  reinterpret_cast<LPVOID*>(&mono_jit_init_version_original));

			ASSERT(status == MH_OK, " Failed to hook onto mono_jit_init_version! Error code: " << status);

			// Enable our hook
			// TODO: Disable it once it's done?
			status = MH_EnableHook(mono_jit_init_version);

			ASSERT(status == MH_OK, " Failed to enable hook for mono_jit_init_version! Error code: " << status);
		}

		~Main()
		{
			// Flush the logger to be sure we got everything
			if(Logger::loggerLoaded)
				Logger::getLogStream().flush();
		}
	};

	// Call main function
	static Main main;
}

// ReSharper disable once CppInconsistentNaming
BOOL WINAPI DllMain(HINSTANCE /* hInstDll */, DWORD reasonForDllLoad, LPVOID /* reserved */)
{
	switch (reasonForDllLoad)
	{
	case DLL_PROCESS_ATTACH:
		break;

	case DLL_PROCESS_DETACH:
		break;

	default:
		break;
	}

	return TRUE;
}