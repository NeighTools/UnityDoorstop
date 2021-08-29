#include "../config.h"
#include "../crt.h"
#include "../logging.h"
#include "../main.h"
#include "hook.h"
#include "proxy/proxy.h"
#include <windows.h>

/**
 * @brief Ensures current working directory is the game folder
 *
 * In some cases (e.g. custom launchers), the CWD (current working directory)
 * is not the same as Unity default. This can break invokable DLLs and even
 * Unity itself. This fix ensures current working directory is the same as
 * application directory and fixes it if not.
 *
 * @return bool_t Whether CWD was changed to match program directory.
 */
bool_t fix_cwd() {
    char_t *app_path = program_path();
    char_t *app_dir = get_folder_name(app_path);
    bool_t fixed_cwd = FALSE;
    char_t *working_dir = get_working_dir();

    if (strcmpi(app_dir, working_dir) != 0) {
        fixed_cwd = TRUE;
        SetCurrentDirectory(app_dir);
    }

    free(app_path);
    free(app_dir);
    free(working_dir);
    return fixed_cwd;
}

#define LOG_FILE_CMD_START L" -logFile \""
#define LOG_FILE_CMD_START_LEN STR_LEN(LOG_FILE_CMD_START)
#define LOG_FILE_CMD_EXTRA 1024

#define LOG_FILE_CMD_END L"\\output_log.txt\""
#define LOG_FILE_CMD_END_LEN STR_LEN(LOG_FILE_CMD_END)

char_t *new_cmdline_args = NULL;
char *new_cmdline_args_narrow = NULL;

LPWSTR WINAPI get_command_line_hook() {
    if (new_cmdline_args)
        return new_cmdline_args;
    return GetCommandLineW();
}

LPSTR WINAPI get_command_line_hook_narrow() {
    if (new_cmdline_args_narrow)
        return new_cmdline_args_narrow;
    return GetCommandLineA();
}

HANDLE stdout_handle;
bool_t WINAPI close_handle_hook(void *handle) {
    if (stdout_handle && handle == stdout_handle)
        return TRUE;
    return CloseHandle(handle);
}

bool_t initialized = FALSE;
void *WINAPI get_proc_address_detour(void *module, char *name) {
#define REDIRECT_INIT(init_name, init_func, target)                            \
    if (lstrcmpA(name, init_name) == 0) {                                      \
        if (!initialized) {                                                    \
            initialized = TRUE;                                                \
            LOG("Got %s at %p\n", init_name, module);                          \
            init_func(module);                                                 \
            LOG("Loaded all runtime functions\n")                              \
        }                                                                      \
        return (void *)(target);                                               \
    }

    REDIRECT_INIT("il2cpp_init", load_il2cpp_funcs, init_il2cpp);
    REDIRECT_INIT("mono_jit_init_version", load_mono_funcs, init_mono);
    REDIRECT_INIT("mono_image_open_from_data_with_name", load_mono_funcs,
                  hook_mono_image_open_from_data_with_name);

    return (void *)GetProcAddress(module, name);
#undef REDIRECT_INIT
}

void redirect_output_log(DoorstopPaths paths) {
    if (!config.redirect_output_log)
        return;

    char_t *cmd = GetCommandLine();
    size_t app_dir_len = strlen(paths.doorstop_filename);
    size_t cmd_len = strlen(cmd);
    size_t new_cmd_len = cmd_len + LOG_FILE_CMD_START_LEN + app_dir_len +
                         LOG_FILE_CMD_END_LEN + LOG_FILE_CMD_EXTRA;
    new_cmdline_args = calloc(new_cmd_len, sizeof(char_t));

    char_t *s = strcpy(new_cmdline_args, cmd);
    s = strcpy(s + cmd_len, LOG_FILE_CMD_START);
    s = strcpy(s + LOG_FILE_CMD_START_LEN - 1, paths.app_dir);
    s = strcpy(s + app_dir_len, LOG_FILE_CMD_END);

    new_cmdline_args_narrow = narrow(new_cmdline_args);

    LOG("Redirected output log\n");
    LOG("CMD line: %s\n", new_cmdline_args);
}

void inject(DoorstopPaths paths) {
    if (!config.enabled) {
        LOG("Doorstop disabled!\n");
        free_logger();
        return;
    }

    LOG("Doorstop enabled!\n");
    HMODULE target_module = GetModuleHandle(TEXT("UnityPlayer"));
    HMODULE app_module = GetModuleHandle(NULL);

    if (!target_module) {
        LOG("No UnityPlayer module found! Using executable as the hook "
            "target.\n");
        target_module = app_module;
    }

    LOG("Installing IAT hooks\n");
    bool_t ok = TRUE;

#define HOOK_SYS(mod, from, to) ok &= iat_hook(mod, "kernel32.dll", &from, &to)

    HOOK_SYS(target_module, GetProcAddress, get_proc_address_detour);
    HOOK_SYS(target_module, CloseHandle, close_handle_hook);
    HOOK_SYS(app_module, GetCommandLineW, get_command_line_hook);
    HOOK_SYS(app_module, GetCommandLineA, get_command_line_hook_narrow);

    // New Unity with separate UnityPlyer.dll
    if (target_module != app_module) {
        HOOK_SYS(target_module, GetCommandLineW, get_command_line_hook);
        HOOK_SYS(target_module, GetCommandLineA, get_command_line_hook_narrow);
    }

#undef HOOK_SYS

    if (!ok) {
        LOG("Failed to install IAT hook!\n");
        free_logger();
    } else {
        LOG("Hooks installed, marking DOORSTOP_DISALBE = TRUE");
        setenv(TEXT("DOORSTOP_DISABLE"), TEXT("TRUE"), TRUE);
    }
}

BOOL WINAPI DllEntry(HINSTANCE hInstDll, DWORD reasonForDllLoad,
                     LPVOID reserved) {
    if (reasonForDllLoad == DLL_PROCESS_DETACH)
        SetEnvironmentVariableW(L"DOORSTOP_DISABLE", NULL);
    if (reasonForDllLoad != DLL_PROCESS_ATTACH)
        return TRUE;

    h_heap = GetProcessHeap();
    bool_t fixed_cwd = fix_cwd();
    DoorstopPaths paths = init(hInstDll, fixed_cwd);

    GetStdHandle(STD_OUTPUT_HANDLE);

#if VERBOSE
    LOG("Standard output handle at %p\n", stdout_handle);
    char_t handle_path[MAX_PATH] = TEXT("\0");
    GetFinalPathNameByHandle(stdout_handle, handle_path, MAX_PATH, 0);
    LOG("Standard output handle path: %s\n", handle_path);
#endif

    load_proxy(paths.doorstop_filename);
    LOG("Proxy loaded\n");

    load_config();
    LOG("Config loaded\n");

    redirect_output_log(paths);

    if (!file_exists(config.target_assembly)) {
        LOG("Could not find target assembly!\n");
        config.enabled = FALSE;
    }

    inject(paths);

    free(paths.app_dir);
    free(paths.app_path);
    free(paths.doorstop_filename);
    free(paths.doorstop_path);
    free(paths.working_dir);

    return TRUE;
}