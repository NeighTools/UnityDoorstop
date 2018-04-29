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
#include <vector>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <Psapi.h>
#include <Shlwapi.h>

#include "../lib/minhook/include/MinHook.h"
#include "logger.h"

#define VERSION "1.0"

// Define a helper macro that creates a typedef and a variable that will hold address to a mono.dll function
#define DEF_MONO_PROC(name, returnType, ...)          \
	typedef returnType (__cdecl * name##_t)(__VA_ARGS__); \
	name##_t name

// A helper macro to load the function address from a library
#define GET_MONO_PROC(name, lib) name = reinterpret_cast<name##_t>(GetProcAddress(lib, #name))


namespace MonoLoader
{
	// =============================================
	// Mono.DLL functions
	// =============================================

	// The functions from mono.dll that we will need
	// Important note:
	// Since we don't have mono headers (too lazy to get them),
	// we'll just use void* for struct pointers.

	// MonoAssembly * mono_domain_assembly_open  (MonoDomain *domain, const char *name);
	DEF_MONO_PROC(mono_domain_assembly_open, void *, void *, const char *);

	// MonoImage * mono_assembly_get_image(MonoAssembly *assembly);
	DEF_MONO_PROC(mono_assembly_get_image, void *, void *);

	// MonoMethodDesc * mono_method_desc_new(const char *name, mono_bool include_namespace);
	DEF_MONO_PROC(mono_method_desc_new, void *, const char *, int32_t);

	// MonoObject * mono_runtime_invoke(MonoMethod *method, void *obj, void **params, MonoObject **exc);
	DEF_MONO_PROC(mono_runtime_invoke, void *, void *, void *, void **, void **);

	// MonoClass * mono_class_from_name(MonoImage *image, const char* name_space, const char *name);
	DEF_MONO_PROC(mono_class_from_name, void *, void *, const char *, const char *);

	// MonoMethod * mono_method_desc_search_in_class(MonoMethodDesc *desc, MonoClass *klass);
	DEF_MONO_PROC(mono_method_desc_search_in_class, void *, void *, void *);

	// MonoDomain * mono_jit_init_version(const char *root_domain_name, const char *runtime_version);
	DEF_MONO_PROC(mono_jit_init_version, void *, const char *, const char *);


	// Our original mono_jit_init_version_original
	mono_jit_init_version_t mono_jit_init_version_original;

	// The hook for mono_jit_init_version
	// We use this since it will always be called once to initialize Mono's JIT
	void* ownMonoJitInitVersion(const char* root_domain_name, const char* runtime_version)
	{
		// Call the original mono_jit_init_version to initialize the Unity Root Domain
		// If this fails, there is a reason beyond us
		// Just let it crash.
		const auto domain = mono_jit_init_version_original(root_domain_name, runtime_version);

		// Load our custom assembly into the domain
		const auto assembly = mono_domain_assembly_open(domain, "UnityPrePatcher\\bin\\PatchLoader.dll");

		if(assembly == nullptr)
		{
			LOG(Logger::getTimeString() << ": Failed to load PatchLoader.dll as a CLR assembly!");
			return domain;
		}

		// Get assembly's image that contains CIL code
		const auto image = mono_assembly_get_image(assembly);

		if (image == nullptr)
		{
			LOG(Logger::getTimeString() << ": Failed to get assembly image for PatchLoader.dll!");
			return domain;
		}

		// Find our Loader class from the assembly
		const auto classDef = mono_class_from_name(image, "PatchLoader", "Loader");

		if (classDef == nullptr)
		{
			LOG(Logger::getTimeString() << ": Failed to load class PatchLoader.Loader!");
			return domain;
		}

		// Create a method descriptor that is used to find the Run() method
		const auto descriptor = mono_method_desc_new("Loader:Run", FALSE);

		// Find Run() method from Loader class
		const auto method = mono_method_desc_search_in_class(descriptor, classDef);

		if (method == nullptr)
		{
			LOG(Logger::getTimeString() << ": Failed to locate Loader.Run() method!");
			return domain;
		}

		// Invoke Run() with no parameters
		mono_runtime_invoke(method, nullptr, nullptr, nullptr);

		return domain;
	}

	static std::wstring getMonoPath()
	{
		// Code to get the name of the Game's Executable
		wchar_t path[MAX_PATH];
		wchar_t name[_MAX_FNAME];

		GetModuleFileName(nullptr, path, sizeof(path));
		_wsplitpath_s(path, nullptr, 0, nullptr, 0, name, sizeof(name), nullptr, 0);

		// The mono.dll should *usually* be in GameName_Data\Mono
		// TODO: A better way to find mono.dll?
		std::wstring monoDll = L".\\";
		monoDll += name;
		monoDll += L"_Data\\Mono\\mono.dll";

		return monoDll;
	}

	struct Main
	{
		Main()
		{
			if(!PathFileExistsW(L".\\UnityPrePatcher"))
			{
				LOG(Logger::getTimeString() << ": No UnityPrePatcher folder found! Aborting...");
				return;
			}

			if(!PathFileExistsW(L".\\UnityPrePatcher\\bin\\PatchLoader.dll"))
			{
				LOG(Logger::getTimeString() << ": No PatchLoader.dll found! Aborting...");
				return;
			}

			const auto monoDllPath = getMonoPath();

			if(!PathFileExistsW(monoDllPath.c_str()))
			{
				LOG(Logger::getTimeString() << ": No mono.dll found from" << monoDllPath << "! Aborting...");
				return;
			}

			// Preload mono into memory so we can start hooking it
			const auto monoLib = LoadLibrary(monoDllPath.c_str());

			if(monoLib == nullptr)
			{
				LOG(Logger::getTimeString() << ": Failed to load mono.dll! Aborting...");
				return;
			}

			// Find and assign all our functions that we are going to use
			GET_MONO_PROC(mono_domain_assembly_open, monoLib);
			GET_MONO_PROC(mono_assembly_get_image, monoLib);
			GET_MONO_PROC(mono_method_desc_new, monoLib);
			GET_MONO_PROC(mono_runtime_invoke, monoLib);
			GET_MONO_PROC(mono_class_from_name, monoLib);
			GET_MONO_PROC(mono_method_desc_search_in_class, monoLib);
			GET_MONO_PROC(mono_jit_init_version, monoLib);

			// Initialize MinHook
			// TODO: Error handling
			auto status = MH_Initialize();

			if(status != MH_OK)
			{
				LOG(Logger::getTimeString() << ": Failed to initialize MinHook! Error code: " << status);
				return;
			}

			// Add a detour to mono_jit_init_version
			status = MH_CreateHook(mono_jit_init_version, &ownMonoJitInitVersion,
			                                  reinterpret_cast<LPVOID*>(&mono_jit_init_version_original));

			if (status != MH_OK)
			{
				LOG(Logger::getTimeString() << ": Failed to hook onto mono_jit_init_version! Error code: " << status);
				return;
			}

			// Enable our hook
			// TODO: Disable it once it's done?
			status = MH_EnableHook(mono_jit_init_version);

			if (status != MH_OK)
			{
				LOG(Logger::getTimeString() << ": Failed to enable hook for mono_jit_init_version! Error code: " << status);
				return;
			}
		}

		~Main()
		{
			// Flush the logger to be sure we got everything
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
	} // switch (reasonForDllLoad)

	return TRUE;
}