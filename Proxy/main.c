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

#include "ver.h"
#include <windows.h>
#include <stdio.h>
#include <Shlwapi.h>
#include <wchar.h>

#include "winapi_util.h"
#include "config.h"
#include "mono.h"
#include "hook.h"
#include "assert_util.h"
#include "proxy.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase; // This is provided by MSVC with the infomration about this DLL

// The hook for mono_jit_init_version
// We use this since it will always be called once to initialize Mono's JIT
void *ownMonoJitInitVersion(const char *root_domain_name, const char *runtime_version)
{
	// Call the original mono_jit_init_version to initialize the Unity Root Domain
	void *domain = mono_jit_init_version(root_domain_name, runtime_version);

	size_t len = WideCharToMultiByte(CP_UTF8, 0, targetAssembly, -1, NULL, 0, NULL, NULL);
	char *dll_path = malloc(sizeof(char) * len);
	WideCharToMultiByte(CP_UTF8, 0, targetAssembly, -1, dll_path, len, NULL, NULL);

	LOGA("Loading assembly: %s\n", dll_path);
	// Load our custom assembly into the domain
	void *assembly = mono_domain_assembly_open(domain, dll_path);

	if (assembly == NULL)
		LOG(L"Failed to load assembly\n");

	free(dll_path);
	ASSERT_SOFT(assembly != NULL, domain);

	// Get assembly's image that contains CIL code
	void *image = mono_assembly_get_image(assembly);
	ASSERT_SOFT(image != NULL, domain);

	// Note: we use the runtime_invoke route since jit_exec will not work on DLLs

	// Create a descriptor for a random Main method
	void *desc = mono_method_desc_new("*:Main", FALSE);

	// Find the first possible Main method in the assembly
	void *method = mono_method_desc_search_in_image(desc, image);
	ASSERT_SOFT(method != NULL, domain);

	void *signature = mono_method_signature(method);

	// Get the number of parameters in the signature
	uint32_t params = mono_signature_get_param_count(signature);

	void **args = NULL;
	wchar_t *app_path = NULL;
	if (params == 1)
	{
		// If there is a parameter, it's most likely a string[].
		// Populate it as follows
		// 0 => path to the game's executable
		// 1 => --doorstop-invoke

		get_module_path(NULL, &app_path, NULL, 0);

		void *exe_path = MONO_STRING(app_path);
		void *doorstop_handle = MONO_STRING(L"--doorstop-invoke");

		void *args_array = mono_array_new(domain, mono_get_string_class(), 2);

		SET_ARRAY_REF(args_array, 0, exe_path);
		SET_ARRAY_REF(args_array, 1, doorstop_handle);

		args = malloc(sizeof(void*) * 1);
		args[0] = args_array;
	}

	LOG(L"Invoking method!\n");
	mono_runtime_invoke(method, NULL, args, NULL);

	if (args != NULL)
	{
		free(app_path);
		free(args);
		args = NULL;
	}

	cleanupConfig();

	free_logger();

	return domain;
}

BOOL initialized = FALSE;

void *hookGetProcAddress(HMODULE module, char const *name)
{
	if (strcmp(name, "mono_jit_init_version") == 0)
	{
		if (!initialized)
		{
			initialized = TRUE;
			LOG(L"Got mono.dll at %p\n", module);
			loadMonoFunctions(module);
		}
		return (void*)&ownMonoJitInitVersion;
	}
	return (void*)GetProcAddress(module, name);
}

BOOL WINAPI DllMain(HINSTANCE hInstDll, DWORD reasonForDllLoad, LPVOID reserved)
{
	if (reasonForDllLoad != DLL_PROCESS_ATTACH)
		return TRUE;

	init_logger();

	LOG(L"Doorstop started!\n");

	wchar_t *dll_path = NULL;
	size_t dll_path_len = get_module_path((HINSTANCE)&__ImageBase, &dll_path, NULL, 0);

	LOG(L"DLL Path: %s\n", dll_path);

	wchar_t *dll_name = get_file_name_no_ext(dll_path, dll_path_len);

	LOG(L"Doorstop DLL Name: %s\n", dll_name);

	size_t hook_name_len = wcslen(dll_name) + 35;
	char *hook_name = malloc(sizeof(char) * hook_name_len); // This is fine, since DLLs must always be ANSI names anyway
	sprintf_s(hook_name, hook_name_len, "%S.ownMonoJitInitVersion", dll_name);

	LOGA("EAT pointer name: %s\n", hook_name);

	loadProxy(dll_name);
	loadConfig();

	// If the loader is disabled, don't inject anything.
	if (enabled)
	{
		LOG(L"Doorstop enabled!\n");
		ASSERT_SOFT(GetFileAttributesW(targetAssembly) != INVALID_FILE_ATTRIBUTES, TRUE);

		LOG(L"Installing IAT hook\n");
		if (!iat_hook(GetModuleHandle(NULL), &GetProcAddress, &hookGetProcAddress))
		{
			LOG(L"Failed to install IAT hook!\n");
		}
	}
	else
	{
		LOG(L"Doorstop dissabled! Freeing resources\n");
		free_logger();
	}

	free(dll_name);
	free(dll_path);
	free(hook_name);

	return TRUE;
}
