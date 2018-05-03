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

#include "logger.h"

#define FINI_WIDE_SUPPORT

#define VERSION "1.3"
#define CONFIG_NAME L"doorstop_config.ini"
#define DEFAULT_LOADER_PATH L"UnityDoorstop"

// A helper for cleaner error logging
#define ASSERT(test, message)                    \
	if(!(test))                                  \
	{                                            \
		Logger::fatalError(message);             \
	}

#define ASSERT_SOFT(test, ...)                   \
	if(!(test))                                  \
	{                                            \
		return __VA_ARGS__;                      \
	}

#include "mono.h"
#include "hook.h"

using namespace Mono;

bool enabled = true;
std::wstring uppPath = DEFAULT_LOADER_PATH;
std::wstring loaderPath;

void loadConfig()
{
	wchar_t curPathProcess[MAX_PATH];
	GetModuleFileNameW(nullptr, curPathProcess, sizeof(curPathProcess));

	wchar_t curPath[_MAX_DIR];
	wchar_t drive[_MAX_DRIVE];
	_wsplitpath_s(curPathProcess, drive, sizeof(drive), curPath, sizeof(curPath), nullptr, 0, nullptr, 0);

	std::wstring iniPath;
	iniPath.append(drive).append(curPath).append(CONFIG_NAME);

	wchar_t enabledString[256] = L"true";
	GetPrivateProfileStringW(L"UnityDoorstop", L"enabled", L"true", enabledString, sizeof(enabledString),
	                         iniPath.c_str());

	if (_wcsnicmp(enabledString, L"true", 4) == 0)
		enabled = true;
	if (_wcsnicmp(enabledString, L"false", 5) == 0)
		enabled = false;

	wchar_t uppPathStr[MAX_PATH] = DEFAULT_LOADER_PATH;
	GetPrivateProfileStringW(L"UnityDoorstop", L"loaderPath", DEFAULT_LOADER_PATH, uppPathStr, sizeof(uppPathStr),
	                         iniPath.c_str());

	uppPath = uppPathStr;
	loaderPath = uppPathStr;
	loaderPath.append(L"\\bin\\UnityDoorstop.Bootstrap.dll");
}

// The hook for mono_jit_init_version
// We use this since it will always be called once to initialize Mono's JIT
void* ownMonoJitInitVersion(const char* root_domain_name, const char* runtime_version)
{
	// Call the original mono_jit_init_version to initialize the Unity Root Domain
	const auto domain = mono_jit_init_version(root_domain_name, runtime_version);

	char dllPathA[MAX_PATH];
	sprintf_s(dllPathA, "%ws", loaderPath.c_str());

	// Load our custom assembly into the domain
	const auto assembly = mono_domain_assembly_open(domain, dllPathA);
	ASSERT_SOFT(assembly != nullptr, domain);

	// Get assembly's image that contains CIL code
	const auto image = mono_assembly_get_image(assembly);
	ASSERT_SOFT(image != nullptr, domain);

	// Find our Loader class from the assembly
	const auto classDef = mono_class_from_name(image, "UnityDoorstop.Bootstrap", "Loader");
	ASSERT_SOFT(classDef != nullptr, domain);

	// Find Run() method from Loader class
	const auto method = mono_class_get_method_from_name(classDef, "Run", -1);
	ASSERT_SOFT(method != nullptr, domain);

	// Invoke Run() with no parameters
	mono_runtime_invoke(method, nullptr, nullptr, nullptr);

	return domain;
}


void Main()
{
	loadConfig();

	// If the loader is disabled, don't inject anything.
	if (!enabled)
		return;

	ASSERT_SOFT(PathFileExistsW(uppPath.c_str()));
	ASSERT_SOFT(PathFileExistsW(loaderPath.c_str()));

	auto monoDllPath = getMonoPath();
	auto monoDllPathStr = monoDllPath.c_str();

	if(!PathFileExistsW(monoDllPathStr))
	{
		monoDllPath = getMonoPath(L"\\EmbedRuntime\\");	// Some versions of Unity have mono placed here instead!
		monoDllPathStr = monoDllPath.c_str();
		ASSERT(PathFileExistsW(monoDllPathStr), L"No mono.dll found! Cannot continue!");
	}

	// Preload mono into memory so we can start hooking it
	const auto monoLib = LoadLibrary(monoDllPathStr);

	ASSERT(monoLib != nullptr, L"Failed to load mono.dll!");

	loadMonoFunctions(monoLib);

	ezHook(monoLib, mono_jit_init_version, "winhttp.ownMonoJitInitVersion");
}

// ReSharper disable once CppInconsistentNaming
BOOL WINAPI DllMain(HINSTANCE /* hInstDll */, DWORD reasonForDllLoad, LPVOID /* reserved */)
{
	switch (reasonForDllLoad)
	{
	case DLL_PROCESS_ATTACH:
		Main();
		break;

	case DLL_PROCESS_DETACH:
		break;

	default:
		break;
	}

	return TRUE;
}
