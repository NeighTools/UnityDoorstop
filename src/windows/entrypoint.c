#include "entrypoint.h"
#include "../bootstrap.h"
#include "../config/config.h"
#include "../crt.h"
#include "../util/logging.h"
#include "../util/paths.h"
#include "hook.h"
#include "proxy/proxy.h"

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

char_t *default_boot_config_path = NULL;

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

HANDLE WINAPI create_file_hook(LPCWSTR lpFileName, DWORD dwDesiredAccess,
                               DWORD dwShareMode,
                               LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                               DWORD dwCreationDisposition,
                               DWORD dwFlagsAndAttributes,
                               HANDLE hTemplateFile) {
    LPCWSTR actual_file_name = lpFileName;

    char_t *normalised_path = calloc(strlen(lpFileName) + 1, sizeof(char_t));
    memset(normalised_path, 0, (strlen(lpFileName) + 1) * sizeof(char_t));
    strcpy(normalised_path, lpFileName);
    for (size_t i = 0; i < strlen(normalised_path); i++) {
        if (normalised_path[i] == L'/') {
            normalised_path[i] = L'\\';
        }
    }

    if (strcmpi(normalised_path, default_boot_config_path) == 0) {
        actual_file_name = config.boot_config_override;
        LOG("Overriding boot.config to %s", actual_file_name);
    }

    free(normalised_path);

    return CreateFileW(actual_file_name, dwDesiredAccess, dwShareMode,
                       lpSecurityAttributes, dwCreationDisposition,
                       dwFlagsAndAttributes, hTemplateFile);
}

HANDLE WINAPI create_file_hook_narrow(
    void *lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) {
    void *actual_file_name = lpFileName;

    char_t *widened_filename = widen(lpFileName);
    char_t *normalised_path =
        calloc(strlen(widened_filename) + 1, sizeof(char_t));
    memset(normalised_path, 0, (strlen(widened_filename) + 1) * sizeof(char_t));
    strcpy(normalised_path, widened_filename);
    free(widened_filename);

    for (size_t i = 0; i < strlen(normalised_path); i++) {
        if (normalised_path[i] == L'/') {
            normalised_path[i] = L'\\';
        }
    }

    if (strcmpi(normalised_path, default_boot_config_path) == 0) {
        char *narrowed_boot_config_override =
            narrow(config.boot_config_override);
        memcpy(actual_file_name, narrowed_boot_config_override,
               strlen(config.boot_config_override));
        free(narrowed_boot_config_override);
        LOG("Overriding boot.config to %s", actual_file_name);
    }

    free(normalised_path);

    return CreateFileA(actual_file_name, dwDesiredAccess, dwShareMode,
                       lpSecurityAttributes, dwCreationDisposition,
                       dwFlagsAndAttributes, hTemplateFile);
}

void capture_mono_path(void *handle) {
    char_t *result;
    get_module_path(handle, &result, NULL, 0);
    setenv(TEXT("DOORSTOP_MONO_LIB_PATH"), result, TRUE);
}

bool_t initialized = FALSE;
void *WINAPI get_proc_address_detour(void *module, char *name) {
#define REDIRECT_INIT(init_name, init_func, target, extra_init)                \
    if (lstrcmpA(name, init_name) == 0) {                                      \
        if (!initialized) {                                                    \
            initialized = TRUE;                                                \
            LOG("Got %S at %p", init_name, module);                            \
            extra_init;                                                        \
            init_func(module);                                                 \
            LOG("Loaded all runtime functions\n")                              \
        }                                                                      \
        return (void *)(target);                                               \
    }

    REDIRECT_INIT("il2cpp_init", load_il2cpp_funcs, init_il2cpp, {});
    REDIRECT_INIT("mono_jit_init_version", load_mono_funcs, init_mono,
                  capture_mono_path(module));
    REDIRECT_INIT("mono_image_open_from_data_with_name", load_mono_funcs,
                  hook_mono_image_open_from_data_with_name,
                  capture_mono_path(module));
    REDIRECT_INIT("mono_jit_parse_options", load_mono_funcs,
                  hook_mono_jit_parse_options, capture_mono_path(module));
    REDIRECT_INIT("mono_debug_init", load_mono_funcs, hook_mono_debug_init,
                  capture_mono_path(module));

    return (void *)GetProcAddress(module, name);
#undef REDIRECT_INIT
}

void redirect_output_log(DoorstopPaths const *paths) {

    if (!config.redirect_output_log)
        return;

    char_t *cmd = GetCommandLine();
    size_t app_dir_len = strlen(paths->app_dir);
    size_t cmd_len = strlen(cmd);
    size_t new_cmd_len = cmd_len + LOG_FILE_CMD_START_LEN + app_dir_len +
                         LOG_FILE_CMD_END_LEN + LOG_FILE_CMD_EXTRA;
    new_cmdline_args = calloc(new_cmd_len, sizeof(char_t));

    char_t *s = strcpy(new_cmdline_args, cmd);
    s = strcpy(s + cmd_len, LOG_FILE_CMD_START);
    s = strcpy(s + LOG_FILE_CMD_START_LEN - 1, paths->app_dir);
    s = strcpy(s + app_dir_len, LOG_FILE_CMD_END);

    new_cmdline_args_narrow = narrow(new_cmdline_args);

    LOG("Redirected output log");
    LOG("CMD line: %s", new_cmdline_args);
}

void inject(DoorstopPaths const *paths) {

    if (!config.enabled) {
        LOG("Doorstop disabled!");
        free_logger();
        return;
    }

    LOG("Doorstop enabled!");
    HMODULE target_module = GetModuleHandle(TEXT("UnityPlayer"));
    HMODULE app_module = GetModuleHandle(NULL);

    if (!target_module) {
        LOG("No UnityPlayer module found! Using executable as the hook "
            "target.\n");
        target_module = app_module;
    }

    LOG("Installing IAT hooks");
    bool_t ok = TRUE;

#define HOOK_SYS(mod, from, to) ok &= iat_hook(mod, "kernel32.dll", &from, &to)

    HOOK_SYS(target_module, GetProcAddress, get_proc_address_detour);
    HOOK_SYS(target_module, CloseHandle, close_handle_hook);
    if (config.boot_config_override) {
        if (file_exists(config.boot_config_override)) {
            default_boot_config_path = calloc(MAX_PATH, sizeof(char_t));
            memset(default_boot_config_path, 0, MAX_PATH * sizeof(char_t));
            strcat(default_boot_config_path, get_working_dir());
            strcat(default_boot_config_path, TEXT("\\"));
            strcat(default_boot_config_path,
                   get_file_name(program_path(), FALSE));
            strcat(default_boot_config_path, TEXT("_Data\\boot.config"));

            HOOK_SYS(target_module, CreateFileW, create_file_hook);
            HOOK_SYS(target_module, CreateFileA, create_file_hook_narrow);
        } else {
            LOG("The boot.config file won't be overriden because the provided "
                "one does not exist: %s",
                config.boot_config_override);
        }
    }

    HOOK_SYS(app_module, GetCommandLineW, get_command_line_hook);
    HOOK_SYS(app_module, GetCommandLineA, get_command_line_hook_narrow);

    // New Unity with separate UnityPlyer.dll
    if (target_module != app_module) {
        HOOK_SYS(target_module, GetCommandLineW, get_command_line_hook);
        HOOK_SYS(target_module, GetCommandLineA, get_command_line_hook_narrow);
    }

#undef HOOK_SYS

    if (!ok) {
        LOG("Failed to install IAT hook!");
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

    init_crt();
    bool_t fixed_cwd = fix_cwd();
    init_logger();
    DoorstopPaths *paths = paths_init(hInstDll, fixed_cwd);

    stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);

// TODO: Some MinGW distributons don't seem to have GetFinalPathNameByHandle
// properly defined
#if VERBOSE && !defined(__MINGW32__)
    LOG("Standard output handle at %p", stdout_handle);
    char_t handle_path[MAX_PATH] = TEXT("\0");
    GetFinalPathNameByHandle(stdout_handle, handle_path, MAX_PATH, 0);
    LOG("Standard output handle path: %s", handle_path);
#endif

    load_proxy(paths->doorstop_filename);
    LOG("Proxy loaded");

    load_config();
    LOG("Config loaded");

    redirect_output_log(paths);

    if (!file_exists(config.target_assembly)) {
        LOG("Could not find target assembly!");
        config.enabled = FALSE;
    }

    inject(paths);

    paths_free(paths);

    return TRUE;
}
