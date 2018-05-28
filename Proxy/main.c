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
#include <stdio.h>
#include <Shlwapi.h>
#include <wchar.h>

#include "config.h"
#include "mono.h"
#include "hook.h"
#include "assert_util.h"
#include "proxy.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase; // This is provided by MSVC with the infomration about this DLL

// The hook for mono_jit_init_version
// We use this since it will always be called once to initialize Mono's JIT
void* ownMonoJitInitVersion(const char* root_domain_name, const char* runtime_version)
{
	// Call the original mono_jit_init_version to initialize the Unity Root Domain
	void* domain = mono_jit_init_version(root_domain_name, runtime_version);

	char dllPathA[MAX_PATH + 1];
	sprintf_s(dllPathA, MAX_PATH + 1, "%S", targetAssembly);

	// Load our custom assembly into the domain
	void* assembly = mono_domain_assembly_open(domain, dllPathA);
	ASSERT_SOFT(assembly != NULL, domain);

	// Get assembly's image that contains CIL code
	void* image = mono_assembly_get_image(assembly);
	ASSERT_SOFT(image != NULL, domain);

	// Note: we use the runtime_invoke route since jit_exec will not work on DLLs

	// Create a descriptor for a random Main method
	void* desc = mono_method_desc_new("*:Main", FALSE);

	// Find the first possible Main method in the assembly
	void* method = mono_method_desc_search_in_image(desc, image);
	ASSERT_SOFT(method != NULL, domain);

	void* signature = mono_method_signature(method);

	// Get the number of parameters in the signature
	uint32_t params = mono_signature_get_param_count(signature);

	void** args = NULL;
	if (params == 1)
	{
		// If there is a parameter, it's most likely a string[].
		// Populate it as follows
		// 0 => path to the game's executable
		// 1 => --doorstop-invoke

		wchar_t path[MAX_PATH + 1];
		GetModuleFileName(NULL, path, sizeof(path));

		void* exe_path = MONO_STRING(path);
		void* doorstop_handle = MONO_STRING(L"--doorstop-invoke");

		void* args_array = mono_array_new(domain, mono_get_string_class(), 2);

		SET_ARRAY_REF(args_array, 0, exe_path);
		SET_ARRAY_REF(args_array, 1, doorstop_handle);

		args = malloc(sizeof(void*) * 1);
		args[0] = args_array;
	}

	mono_runtime_invoke(method, NULL, args, NULL);

	if (args != NULL)
	{
		free(args);
		args = NULL;
	}

	return domain;
}

BOOL WINAPI DllMain(HINSTANCE hInstDll, DWORD reasonForDllLoad, LPVOID reserved)
{
	if (reasonForDllLoad != DLL_PROCESS_ATTACH)
		return TRUE;

	wchar_t path[MAX_PATH + 1]; // Path to this DLL
	wchar_t dllName[_MAX_FNAME + 1]; // The name of the DLL
	char hookName[_MAX_FNAME + 30]; //Name of the hook method that will be added to mono's EAT

	GetModuleFileName((HINSTANCE)&__ImageBase, path, MAX_PATH + 1);
	_wsplitpath_s(path, NULL, 0, NULL, 0, dllName, _MAX_FNAME + 1, NULL, 0);

	sprintf_s(hookName, _MAX_FNAME + 30, "%S.ownMonoJitInitVersion", dllName);

	loadProxy(dllName);
	loadConfig();

	// If the loader is disabled, don't inject anything.
	if (!enabled)
		return TRUE;

	ASSERT_SOFT(PathFileExistsW(targetAssembly), TRUE);

	HMODULE monoLib = initMonoLib();
	ezHook(monoLib, mono_jit_init_version, hookName);

	return TRUE;
}
