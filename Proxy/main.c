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
#include "il2cpp.h"
#include "hook.h"
#include "assert_util.h"
#include "proxy.h"

static BOOL initialized = FALSE;

// The hook for mono_jit_init_version
// We use this since it will always be called once to initialize Mono's JIT
void doorstop_invoke(void *domain) {
    VERBOSE_ONLY({
        HANDLE stdout = GetStdHandle(STD_OUTPUT_HANDLE);
        LOG("STDOUT handle at %p\n", stdout);
        char handlepath[2046] = "\0";
        GetFinalPathNameByHandleA(stdout, handlepath, 2046, 0);
        LOG("STDOUT handle path: %s\n", handlepath);
        });

    if (!config.ignore_disabled_env && GetEnvironmentVariableW(L"DOORSTOP_INITIALIZED", NULL, 0) != 0) {
        LOG("DOORSTOP_INITIALIZED is set! Skipping!");
        free_logger();
        return;
    }
    SetEnvironmentVariableW(L"DOORSTOP_INITIALIZED", L"TRUE");

    mono_thread_set_main(mono_thread_current());

    if (mono_domain_set_config) {
#define CONFIG_EXT L".config"

        wchar_t *exe_path = NULL;
        const size_t real_len = get_module_path(NULL, &exe_path, NULL, STR_LEN(CONFIG_EXT));
        wchar_t *folder_name = get_folder_name(exe_path, real_len, TRUE);
        wmemcpy(exe_path + real_len, CONFIG_EXT, STR_LEN(CONFIG_EXT));

        char *exe_path_n = narrow(exe_path);
        char *folder_path_n = narrow(folder_name);

        LOG("Setting config paths: base dir: %s; config path: %s\n", folder_path_n, exe_path_n);

        // TODO: Doesn't yet work with .NET mono but it is needed by Unity mono
        //mono_domain_set_config(domain, folder_path_n, exe_path_n);
        LOG("Done setting config paths\n");

        free(exe_path);
        free(folder_name);
        free(exe_path_n);
        free(folder_path_n);

#undef CONFIG_EXT
    }

    // Set target assembly as an environment variable for use in the managed world
    SetEnvironmentVariableW(L"DOORSTOP_INVOKE_DLL_PATH", config.target_assembly);

    // Set path to managed folder dir as an env variable
    char *assembly_dir = mono_assembly_getrootdir();
    LOG("Assembly dir: %s\n", assembly_dir);

    wchar_t *wide_assembly_dir = widen(assembly_dir);
    SetEnvironmentVariableW(L"DOORSTOP_MANAGED_FOLDER_DIR", wide_assembly_dir);
    free(wide_assembly_dir);

    const int len = WideCharToMultiByte(CP_UTF8, 0, config.target_assembly, -1, NULL, 0, NULL, NULL);
    char *dll_path = malloc(sizeof(char) * len);
    WideCharToMultiByte(CP_UTF8, 0, config.target_assembly, -1, dll_path, len, NULL, NULL);

    wchar_t *app_path = NULL;
    get_module_path(NULL, &app_path, NULL, 0);
    SetEnvironmentVariableW(L"DOORSTOP_PROCESS_PATH", app_path);

    LOG("Loading assembly: %s\n", dll_path);
    // Load our custom assembly into the domain
    void *assembly = mono_domain_assembly_open(domain, dll_path);

    if (assembly == NULL)
    LOG("Failed to load assembly\n");

    free(dll_path);
    ASSERT_SOFT(assembly != NULL);

    // Get assembly's image that contains CIL code
    void *image = mono_assembly_get_image(assembly);
    ASSERT_SOFT(image != NULL);

    // Create a descriptor for a random Main method
    void *desc = mono_method_desc_new("*:Main", FALSE);

    // Find the first possible Main method in the assembly
    void *method = mono_method_desc_search_in_image(desc, image);
    ASSERT_SOFT(method != NULL);

    void *signature = mono_method_signature(method);

    // Get the number of parameters in the signature
    UINT32 params = mono_signature_get_param_count(signature);

    void **args = NULL;
    if (params == 1) {
        // If there is a parameter, it's most likely a string[].
        void *args_array = mono_array_new(domain, mono_get_string_class(), 0);
        args = malloc(sizeof(void*) * 1);
        args[0] = args_array;
    }

    // Note: we use the runtime_invoke route since jit_exec will not work on DLLs
    LOG("Invoking method %p\n", method);
    void *exc = NULL;
    mono_runtime_invoke(method, NULL, args, &exc);
    VERBOSE_ONLY({
        if (exc != NULL) {
            LOG("Error invoking code!\n");
            void* ex_klass = mono_get_exception_class();
            void* to_string_desc = mono_method_desc_new("*:ToString()", FALSE);
            void* to_string_method = mono_method_desc_search_in_class(to_string_desc, ex_klass);
            mono_method_desc_free(to_string_desc);
            if (to_string_method) {
                void* real_to_string_method = mono_object_get_virtual_method(exc, to_string_method);
                void* exc2 = NULL;
                void* str = mono_runtime_invoke(real_to_string_method, exc, NULL, &exc2);
                if (!exc2) {
                    char* exc_str = mono_string_to_utf8(str);
                    LOG("Error message: %s\n", exc_str);
                }
            }
        }
    })
    LOG("Done!\n");

    // cleanup method_desc
    mono_method_desc_free(desc);

    if (args != NULL) {
        free(app_path);
        free(args);
        args = NULL;
    }

    free_logger();
}

int init_doorstop_il2cpp(const char *domain_name) {
    LOG("Starting IL2CPP domain \"%s\"\n", domain_name);
    const int orig_result = il2cpp_init(domain_name);

    if (!config.mono_lib_dir || !config.mono_corlib_dir || !config.mono_config_dir) {
        LOG("No mono paths set! Skipping loading...\n");
        return orig_result;
    }

    wchar_t *mono_lib_dir = get_full_path(config.mono_lib_dir);
    wchar_t *mono_corlib_dir = get_full_path(config.mono_corlib_dir);
    wchar_t *mono_config_dir = get_full_path(config.mono_config_dir);

    LOG("Mono lib: %S\n", mono_lib_dir);
    LOG("Mono mscorlib dir: %S\n", mono_corlib_dir);
    LOG("Mono confgi dir: %S\n", mono_config_dir);

    if (!file_exists(mono_lib_dir) || !folder_exists(mono_corlib_dir) || !folder_exists(mono_config_dir)) {
        LOG("Mono startup dirs are not set up, skipping invoking Doorstop\n");
        return orig_result;
    }

    const HMODULE mono_module = LoadLibraryW(mono_lib_dir);
    LOG("Loaded mono.dll: %p\n", mono_module);
    if (!mono_module) {
        LOG("Failed to load mono.dll! Skipping!");
        return orig_result;
    }

    load_mono_functions(mono_module);
    LOG("Loaded mono.dll functions\n");

    char *mono_corlib_dir_narrow = narrow(mono_corlib_dir);
    char *mono_config_dir_narrow = narrow(mono_config_dir);
    mono_set_dirs(mono_corlib_dir_narrow, mono_config_dir_narrow);
    mono_set_assemblies_path(mono_corlib_dir_narrow);
    if (mono_config_parse)
        mono_config_parse(NULL);

    if (config.mono_debug_enabled) {
        const char* debugger_option = "--debugger-agent=transport=dt_socket,server=y,address=";
        size_t len_debugger_option = strlen(debugger_option);
        char* debug_address = narrow(config.mono_debug_address);
        size_t len_debug_address = strlen(debug_address);
        const char* no_suspend = ",suspend=n";
        size_t len_no_suspend = strlen(no_suspend);

        size_t len_option = len_debugger_option + len_debug_address;
        if (!config.mono_debug_suspend) {
            len_option += len_no_suspend;
        }
        char* option = malloc((len_option + 1) * sizeof(char));
        memcpy(option, debugger_option, len_debugger_option);
        memcpy(option + len_debugger_option, debug_address, len_debug_address);
        if (!config.mono_debug_suspend) {
            memcpy(option + len_debugger_option + len_debug_address, no_suspend, len_no_suspend);
        }
        option[len_option + 1] = '\0';

        const char* options[] = {
            option,
            "--soft-breakpoints"
        };
        mono_jit_parse_options(2, options);
        mono_debug_init(MONO_DEBUG_FORMAT_MONO);
        LOG("Mono debugger listening on %s\n", debug_address);

        free(option);
        free(debug_address);
    }

    void *domain = mono_jit_init_version("Doorstop Root Domain", NULL);
    if (config.mono_debug_enabled) {
        mono_debug_domain_create(domain);
    }
    LOG("Created domain: %p\n", domain);

    doorstop_invoke(domain);

    return orig_result;
}

void *init_doorstop_mono(const char *root_domain_name, const char *runtime_version) {
    LOG("Starting Mono domain \"%s\"\n", root_domain_name);
    char* root_dir = mono_assembly_getrootdir();
    if (config.mono_dll_search_path_override) {
        size_t len = strlen(root_dir);
        LOG("Current root dir: %s\n", root_dir);

        wchar_t *mono_bcl_root_dir_full = get_full_path(config.mono_dll_search_path_override);
        char *mono_bcl_root_dir_narrow = narrow(mono_bcl_root_dir_full);
        size_t len_bcl = strlen(mono_bcl_root_dir_narrow);
        LOG("New root path: %s\n", mono_bcl_root_dir_narrow);

        char *search_path = (char*)calloc(len + len_bcl + 2, sizeof(char));
        memcpy(search_path, mono_bcl_root_dir_narrow, len_bcl);
        search_path[len_bcl] = ';';
        memcpy(search_path + len_bcl + 1, root_dir, len);

        LOG("Search path: %s\n", search_path);
        mono_set_assemblies_path(search_path);
        wchar_t* search_path_wide = widen(search_path);
        SetEnvironmentVariableW(L"DOORSTOP_DLL_SEARCH_DIRS", search_path_wide);
        free(search_path_wide);
        free(search_path);
        free(mono_bcl_root_dir_narrow);
        free(mono_bcl_root_dir_full);
    } else {
        wchar_t* root_dir_wide = widen(root_dir);
        SetEnvironmentVariableW(L"DOORSTOP_DLL_SEARCH_DIRS", root_dir_wide);
        free(root_dir_wide);
    }
    void *domain = mono_jit_init_version(root_domain_name, runtime_version);
    doorstop_invoke(domain);
    return domain;
}

void *hook_mono_image_open_from_data_with_name(void *data, DWORD data_len, int need_copy, void *status, int refonly,
                                               const char *name) {
    void *result = NULL;
    if (config.mono_dll_search_path_override) {
        wchar_t *name_wide = widen(name);
        wchar_t *name_file = get_file_name(name_wide, strlen(name), TRUE);
        free(name_wide);

        size_t name_file_len = wcslen(name_file);
        size_t bcl_root_len = wcslen(config.mono_dll_search_path_override);

        wchar_t *new_full_path = calloc(name_file_len + bcl_root_len + 2, sizeof(wchar_t));
        wmemcpy(new_full_path, config.mono_dll_search_path_override, bcl_root_len);
        new_full_path[bcl_root_len] = L'\\';
        wmemcpy(new_full_path + bcl_root_len + 1, name_file, name_file_len);
        free(name_file);

        if (file_exists(new_full_path)) {
            HANDLE h = CreateFileW(new_full_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                                   FILE_ATTRIBUTE_NORMAL, NULL);
            DWORD size = GetFileSize(h, NULL);
            void *buf = malloc(size);
            BOOL res = ReadFile(h, buf, size, &size, NULL);
            CloseHandle(h);
            result = mono_image_open_from_data_with_name(buf, size, need_copy, status, refonly, name);
            if (need_copy)
                free(buf);
        }
        free(new_full_path);
    }

    if (!result) { result = mono_image_open_from_data_with_name(data, data_len, need_copy, status, refonly, name); }
    return result;
}

void * WINAPI get_proc_address_detour(HMODULE module, char const *name) {
#define REDIRECT_INIT(init_name, init_func, target)                 \
    if (lstrcmpA(name, init_name) == 0) {                           \
        if (!initialized) {                                         \
            initialized = TRUE;                                     \
            LOG("Got %s at %p\n", init_name, module);               \
            init_func(module);                                      \
            LOG("Loaded all runtime functions\n")                   \
        }                                                           \
        return (void*)(target);                                     \
    }
    REDIRECT_INIT("il2cpp_init", load_il2cpp_functions, init_doorstop_il2cpp);
    REDIRECT_INIT("mono_jit_init_version", load_mono_functions, init_doorstop_mono);
    REDIRECT_INIT("mono_image_open_from_data_with_name", load_mono_functions, hook_mono_image_open_from_data_with_name);
    return (void*)GetProcAddress(module, name);

#undef REDIRECT_INIT
}

HANDLE stdout_handle = NULL;
BOOL WINAPI close_handle_hook(HANDLE handle) {
    if (stdout_handle && handle == stdout_handle)
        return TRUE;
    return CloseHandle(handle);
}

wchar_t *new_cmdline_args = NULL;
char *cmdline_args_narrow = NULL;
LPWSTR WINAPI get_command_line_hook() {
    if (new_cmdline_args)
        return new_cmdline_args;
    return GetCommandLineW();
}

LPSTR WINAPI get_command_line_hook_narrow() {
    if (cmdline_args_narrow)
        return cmdline_args_narrow;
    return GetCommandLineA();
}

#define LOG_FILE_CMD_START L" -logFile \""
#define LOG_FILE_CMD_START_LEN STR_LEN(LOG_FILE_CMD_START)

#define LOG_FILE_CMD_END L"\\output_log.txt\""
#define LOG_FILE_CMD_END_LEN STR_LEN(LOG_FILE_CMD_END)

// ReSharper disable once CppParameterNeverUsed
BOOL WINAPI DllEntry(HINSTANCE hInstDll, DWORD reasonForDllLoad, LPVOID reserved) {
    if (reasonForDllLoad == DLL_PROCESS_DETACH)
        SetEnvironmentVariableW(L"DOORSTOP_DISABLE", NULL);
    if (reasonForDllLoad != DLL_PROCESS_ATTACH)
        return TRUE;

    h_heap = GetProcessHeap();

    wchar_t *app_path = NULL;
    const size_t app_path_len = get_module_path(NULL, &app_path, NULL, 0);
    wchar_t *app_dir = get_folder_name(app_path, app_path_len, FALSE);
    BOOL fixedCWD = FALSE;

    wchar_t *working_dir = NULL;
    get_working_dir(&working_dir);

    if (lstrcmpiW(app_dir, working_dir) != 0) {
        fixedCWD = TRUE;
        SetCurrentDirectoryW(app_dir);
    }

    init_logger();

    LOG("Doorstop started!\n");

    LOG("EXE Path: %S\n", app_path);
    LOG("App dir: %S\n", app_dir);
    LOG("Working dir: %S\n", working_dir);

    if (fixedCWD) { LOG("WARNING: Working directory is not the same as app directory! Fixing working directory!\n"); }

    stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);

    VERBOSE_ONLY({
        LOG("STDOUT handle at %p\n", stdout_handle);
        char handlepath[2046] = "\0";
        GetFinalPathNameByHandleA(stdout_handle, handlepath, 2046, 0);
        LOG("Pointer to GetFinalPathNameByHandleA %p\n", &GetFinalPathNameByHandleA);
        LOG("STDOUT handle path: %s\n", handlepath);
        });

    wchar_t *dll_path = NULL;
    const size_t dll_path_len = get_module_path(hInstDll, &dll_path, NULL, 0);
    LOG("DLL Path: %S\n", dll_path);

    wchar_t *dll_name = get_file_name(dll_path, dll_path_len, FALSE);
    LOG("Doorstop DLL Name: %S\n", dll_name);

    load_proxy(dll_name);
    LOG("Proxy loaded\n");
    load_config();
    LOG("Config loaded\n");

    if (config.redirect_output_log) {
        wchar_t *cmd = GetCommandLineW();
        const size_t app_dir_len = wcslen(app_dir);
        const size_t cmd_len = wcslen(cmd);
        const size_t new_cmd_size = cmd_len + LOG_FILE_CMD_START_LEN + app_path_len + LOG_FILE_CMD_END_LEN + 1024;
        new_cmdline_args = calloc(new_cmd_size, sizeof(wchar_t));
        // Add some padding in case some hook does the "conventional" replace
        wmemcpy(new_cmdline_args, cmd, cmd_len);
        wmemcpy(new_cmdline_args + cmd_len, LOG_FILE_CMD_START, LOG_FILE_CMD_START_LEN);
        wmemcpy(new_cmdline_args + cmd_len + LOG_FILE_CMD_START_LEN - 1, app_dir, app_dir_len);
        wmemcpy(new_cmdline_args + cmd_len + LOG_FILE_CMD_START_LEN + app_dir_len - 1, LOG_FILE_CMD_END,
                LOG_FILE_CMD_END_LEN);
        cmdline_args_narrow = narrow(new_cmdline_args);

        LOG("Redirected output log!\n");
        LOG("CMDLine: %S\n", new_cmdline_args);
    }

    if (GetFileAttributesW(config.target_assembly) == INVALID_FILE_ATTRIBUTES) {
        LOG("Could not find target assembly! Cannot enable!");
        config.enabled = FALSE;
    }

    // If the loader is disabled, don't inject anything.
    if (config.enabled) {
        LOG("Doorstop enabled!\n");

        HMODULE target_module = GetModuleHandleA("UnityPlayer");
        const HMODULE app_module = GetModuleHandleA(NULL);

        if (!target_module) {
            LOG("No UnityPlayer.dll; using EXE as the hook target.");
            target_module = app_module;
        }

        LOG("Installing IAT hook\n");
        if (!iat_hook(target_module, "kernel32.dll", &GetProcAddress, &get_proc_address_detour) ||
            !iat_hook(target_module, "kernel32.dll", &CloseHandle, &close_handle_hook) ||
            !iat_hook(app_module, "kernel32.dll", &GetCommandLineW, &get_command_line_hook) ||
            !iat_hook(app_module, "kernel32.dll", &GetCommandLineA, &get_command_line_hook_narrow) ||
            target_module != app_module && (
                !iat_hook(target_module, "kernel32.dll", &GetCommandLineW, &get_command_line_hook) ||
                !iat_hook(target_module, "kernel32.dll", &GetCommandLineA, &get_command_line_hook_narrow)
            )) {
            LOG("Failed to install IAT hook!\n");
            free_logger();
        }
        else {
            LOG("Hook installed!!\n");
            // Prevent other instances of Doorstop running in the same process
            SetEnvironmentVariableW(L"DOORSTOP_DISABLE", L"TRUE");
        }
    }
    else {
        LOG("Doorstop disabled! freeing resources\n");
        free_logger();
    }

    free(dll_name);
    free(dll_path);
    free(app_dir);
    free(app_path);
    free(working_dir);

    return TRUE;
}
