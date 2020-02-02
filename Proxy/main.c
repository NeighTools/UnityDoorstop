/*
 * main.cpp -- The main "entry point" and the main logic of the DLL.
 *
 * Here, we do the main magic of the whole DLL
 * 
 * The main procedure goes as follows:
 * 0. Initialize the proxy functions
 * 1. Read configuration (whether to enable Doorstop, what .NET assembly to execute, etc)
 * 2. Find the Unity player module (it's either the game EXE or UnityPlayer.dll)
 * 3. Install IAT hook to GetProcAddress into the Unity player
 * 4. When Unity tries to resolve mono_jit_init_version, grab the mono module and return the address to init_doorstop
 * 
 * Then, the loader waits until Unity creates its root domain for mono (which is done with mono_jit_init_version).
 * 
 * Inside mono_jit_init_version hook (i.e. init_doorstop):
 * 1. Call the original mono_jit_init_version to get the Unity root domain
 * 2. Load the .NET assembly we want to run into the root domain
 * 3. Find Main() method inside the target assembly and invoke it
 * 
 * Rest of the work is done on the managed side.
 *
 */

#include "winapi_util.h"
#include <windows.h>

#include "config.h"
#include "mono.h"
#include "hook.h"
#include "assert_util.h"
#include "proxy.h"

// The hook for mono_jit_init_version
// We use this since it will always be called once to initialize Mono's JIT
void *init_doorstop(const char *root_domain_name, const char *runtime_version)
{
	LOG("Starting mono domain\n");

#if _VERBOSE
	HANDLE stdout = GetStdHandle(STD_OUTPUT_HANDLE);
	LOG("STDOUT handle at %p\n", stdout);
	char handlepath[2046] = "\0";
	GetFinalPathNameByHandleA(stdout, handlepath, 2046, 0);
	LOG("STDOUT handle path: %s\n", handlepath);
#endif

	// Call the original mono_jit_init_version to initialize the Unity Root Domain
	void *domain = mono_jit_init_version(root_domain_name, runtime_version);

	mono_thread_set_main(mono_thread_current());

	if (mono_domain_set_config)
	{
#define CONFIG_EXT L".config"

		wchar_t *exe_path = NULL;
		const size_t real_len = get_module_path(NULL, &exe_path, NULL, STR_LEN(CONFIG_EXT));
		wchar_t *folder_name = get_folder_name(exe_path, real_len, TRUE);
		wmemcpy(exe_path + real_len, CONFIG_EXT, STR_LEN(CONFIG_EXT));

		char *exe_path_n = narrow(exe_path);
		char *folder_path_n = narrow(folder_name);

		LOG("Setting config paths: base dir: %s; config path: %s\n", folder_path_n, exe_path_n);

		mono_domain_set_config(domain, folder_path_n, exe_path_n);

		memfree(exe_path);
		memfree(folder_name);
		memfree(exe_path_n);
		memfree(folder_path_n);

#undef CONFIG_EXT
	}

	// Set target assembly as an environment variable for use in the managed world
	SetEnvironmentVariableW(L"DOORSTOP_INVOKE_DLL_PATH", target_assembly);

	// Set path to managed folder dir as an env variable
	char *assembly_dir = mono_assembly_getrootdir();
	LOG("Assembly dir: %s\n", assembly_dir);

	wchar_t *wide_assembly_dir = widen(assembly_dir);
	SetEnvironmentVariableW(L"DOORSTOP_MANAGED_FOLDER_DIR", wide_assembly_dir);
	memfree(wide_assembly_dir);

	size_t len = WideCharToMultiByte(CP_UTF8, 0, target_assembly, -1, NULL, 0, NULL, NULL);
	char *dll_path = memalloc(sizeof(char) * len);
	WideCharToMultiByte(CP_UTF8, 0, target_assembly, -1, dll_path, len, NULL, NULL);

	LOG("Loading assembly: %s\n", dll_path);
	// Load our custom assembly into the domain
	void *assembly = mono_domain_assembly_open(domain, dll_path);

	if (assembly == NULL)
	LOG("Failed to load assembly\n");

	memfree(dll_path);
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
	UINT32 params = mono_signature_get_param_count(signature);

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

		args = memalloc(sizeof(void*) * 1);
		args[0] = args_array;
	}

	LOG("Invoking method!\n");
	mono_runtime_invoke(method, NULL, args, NULL);

	// cleanup method_desc
	mono_method_desc_free(desc);

	if (args != NULL)
	{
		memfree(app_path);
		memfree(args);
		args = NULL;
	}

	cleanup_config();

	free_logger();

	return domain;
}

BOOL initialized = FALSE;

void * WINAPI get_proc_address_detour(HMODULE module, char const *name)
{
	if (lstrcmpA(name, "mono_jit_init_version") == 0)
	{
		if (!initialized)
		{
			initialized = TRUE;
			LOG("Got mono.dll at %p\n", module);
			load_mono_functions(module);
			LOG("Loaded all mono.dll functions\n");
		}
		return (void*)& init_doorstop;
	}

	return (void*)GetProcAddress(module, name);
}

HANDLE stdoutHandle = NULL;

BOOL WINAPI close_handle_hook(HANDLE handle)
{
	if (stdoutHandle && handle == stdoutHandle)
		return TRUE;
	return CloseHandle(handle);
}


wchar_t *new_cmdline_args = NULL;

LPWSTR WINAPI get_command_line_hook()
{
	if (new_cmdline_args)
		return new_cmdline_args;
	return GetCommandLineW();
}

#define LOG_FILE_CMD_START L" -logFile \""
#define LOG_FILE_CMD_START_LEN STR_LEN(LOG_FILE_CMD_START)

#define LOG_FILE_CMD_END L"\\output_log.txt\""
#define LOG_FILE_CMD_END_LEN STR_LEN(LOG_FILE_CMD_END)

BOOL WINAPI DllEntry(HINSTANCE hInstDll, DWORD reasonForDllLoad, LPVOID reserved)
{
	if (reasonForDllLoad != DLL_PROCESS_ATTACH)
		return TRUE;

	hHeap = GetProcessHeap();

	wchar_t *app_path = NULL;
	size_t app_path_len = get_module_path(NULL, &app_path, NULL, 0);
	wchar_t *app_dir = get_folder_name(app_path, app_path_len, FALSE);
	BOOL fixedCWD = FALSE;

	wchar_t *working_dir = NULL;
	get_working_dir(&working_dir);

	if (lstrcmpiW(app_dir, working_dir) != 0)
	{
		fixedCWD = TRUE;
		SetCurrentDirectoryW(app_dir);
	}

	init_logger();

	LOG("Doorstop started!\n");

	LOG("App dir: %S\n", app_dir);
	LOG("Working dir: %S\n", working_dir);

	if (fixedCWD)
	LOG("WARNING: Working directory is not the same as app directory! Fixing working directory!\n");

	stdoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);

#if _VERBOSE
	LOG("STDOUT handle at %p\n", stdoutHandle);
	char handlepath[2046] = "\0";
	GetFinalPathNameByHandleA(stdoutHandle, handlepath, 2046, 0);
	LOG("Pointer to GetFinalPathNameByHandleA %p\n", &GetFinalPathNameByHandleA);
	LOG("STDOUT handle path: %s\n", handlepath);
#endif

	wchar_t *dll_path = NULL;
	size_t dll_path_len = get_module_path(hInstDll, &dll_path, NULL, 0);

	LOG("DLL Path: %S\n", dll_path);

#if _VERBOSE
	wchar_t* exe_path = NULL;
	get_module_path(NULL, &exe_path, NULL, 0);

	LOG("EXE Path: %S\n", exe_path);
#endif

	wchar_t *dll_name = get_file_name_no_ext(dll_path, dll_path_len);

	LOG("Doorstop DLL Name: %S\n", dll_name);

	load_proxy(dll_name);
	load_config();

	if (redirect_output_log)
	{
		wchar_t *cmd = GetCommandLineW();
		size_t app_dir_len = wcslen(app_dir);
		size_t cmd_len = wcslen(cmd);
		size_t new_cmd_size = cmd_len + LOG_FILE_CMD_START_LEN + app_path_len + LOG_FILE_CMD_END_LEN + 1024;
		new_cmdline_args = memcalloc(sizeof(wchar_t) * new_cmd_size);
		// Add some padding in case some hook does the "conventional" replace
		wmemcpy(new_cmdline_args, cmd, cmd_len);
		wmemcpy(new_cmdline_args + cmd_len, LOG_FILE_CMD_START, LOG_FILE_CMD_START_LEN);
		wmemcpy(new_cmdline_args + cmd_len + LOG_FILE_CMD_START_LEN - 1, app_dir, app_dir_len);
		wmemcpy(new_cmdline_args + cmd_len + LOG_FILE_CMD_START_LEN + app_dir_len - 1, LOG_FILE_CMD_END,
		        LOG_FILE_CMD_END_LEN);

		LOG("Redirected output log!\n");
		LOG("CMDLine: %S\n", new_cmdline_args);
	}

	if (GetFileAttributesW(target_assembly) == INVALID_FILE_ATTRIBUTES)
	{
		LOG("Could not find target assembly! Cannot enable!");
		enabled = FALSE;
	}
	
	// If the loader is disabled, don't inject anything.
	if (enabled)
	{
		LOG("Doorstop enabled!\n");

		HMODULE targetModule = GetModuleHandleA("UnityPlayer");

		if (targetModule == NULL)
		{
			LOG("No UnityPlayer.dll; using EXE as the hook target.");
			targetModule = GetModuleHandleA(NULL);
		}

		LOG("Installing IAT hook\n");
		if (!iat_hook(targetModule, "kernel32.dll", &GetProcAddress, &get_proc_address_detour) ||
			!iat_hook(targetModule, "kernel32.dll", &CloseHandle, &close_handle_hook) ||
			!iat_hook(targetModule, "kernel32.dll", &GetCommandLineW, &get_command_line_hook))
		{
			LOG("Failed to install IAT hook!\n");
			free_logger();
		}
		else
		{
			LOG("Hook installed!\n");

			// Disable Doorstop to ensure we only run one instance of it
			SetEnvironmentVariableW(L"DOORSTOP_DISABLE", L"TRUE");
		}
	}
	else
	{
		LOG("Doorstop disabled! memfreeing resources\n");
		free_logger();
	}

	memfree(dll_name);
	memfree(dll_path);
	memfree(app_dir);
	memfree(app_path);
	memfree(working_dir);

	return TRUE;
}
