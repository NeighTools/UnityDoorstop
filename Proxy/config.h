#pragma once

#include <windows.h>
#include <shellapi.h>
#include "winapi_util.h"
#include "assert_util.h"

#define CONFIG_NAME L"doorstop_config.ini"
#define DEFAULT_TARGET_ASSEMBLY L"Doorstop.dll"
#define EXE_EXTENSION_LENGTH 4

BOOL enabled = FALSE;
BOOL redirect_output_log = FALSE;
wchar_t *target_assembly = NULL;

#define STR_EQUAL(str1, str2) (lstrcmpiW(str1, str2) == 0)

inline void init_config_file() {
    if (GetFileAttributesW(CONFIG_NAME) == INVALID_FILE_ATTRIBUTES)
        return;

    wchar_t* config_path = get_full_path(CONFIG_NAME);

    wchar_t enabled_string[256] = L"true";
    GetPrivateProfileStringW(L"UnityDoorstop", L"enabled", L"true", enabled_string, 256, config_path);

    if (STR_EQUAL(enabled_string, L"true"))
        enabled = TRUE;
    else if (STR_EQUAL(enabled_string, L"false"))
        enabled = FALSE;

    wmemset(enabled_string, '\0', 256);
    GetPrivateProfileStringW(L"UnityDoorstop", L"redirectOutputLog", L"false", enabled_string, 256, config_path);

    if (STR_EQUAL(enabled_string, L"true"))
        redirect_output_log = TRUE;
    else if (STR_EQUAL(enabled_string, L"false"))
        redirect_output_log = FALSE;

    wchar_t *tmp = get_ini_entry(config_path, L"UnityDoorstop", L"targetAssembly", DEFAULT_TARGET_ASSEMBLY);
    target_assembly = get_full_path(tmp);

    LOG("Config; Target assembly: %S\n", target_assembly);

    free(tmp);
    free(config_path);
}

inline void init_cmd_args() {
    wchar_t *args = GetCommandLineW();
    int argc = 0;
    wchar_t **argv = CommandLineToArgvW(args, &argc);

#define IS_ARGUMENT(arg_name) STR_EQUAL(arg, arg_name) && i < argc

    for (int i = 0; i < argc; i++) {
        wchar_t *arg = argv[i];
        if (IS_ARGUMENT(L"--doorstop-enable")) {
            wchar_t *par = argv[++i];

            if (STR_EQUAL(par, L"true"))
                enabled = TRUE;
            else if (STR_EQUAL(par, L"false"))
                enabled = FALSE;
        }
        else if (IS_ARGUMENT(L"--redirect-output-log")) {
            wchar_t *par = argv[++i];

            if (STR_EQUAL(par, L"true"))
                redirect_output_log = TRUE;
            else if (STR_EQUAL(par, L"false"))
                redirect_output_log = FALSE;
        }
        else if (IS_ARGUMENT(L"--doorstop-target")) {
            if (target_assembly != NULL)
                free(target_assembly);
            const size_t len = wcslen(argv[i + 1]) + 1;
            target_assembly = malloc(sizeof(wchar_t) * len);
            wmemcpy(target_assembly, argv[++i], len);
            LOG("Args; Target assembly: %S\n", target_assembly);
        }
    }

    LocalFree(argv);
}

inline void init_env_vars() {
    if (GetEnvironmentVariableW(L"DOORSTOP_DISABLE", NULL, 0) != 0) {
        LOG("DOORSTOP_DISABLE is set! Disabling Doorstop!\n");
        enabled = FALSE;
    }
}

inline void load_config() {
    init_config_file();
    init_cmd_args();
    init_env_vars();
}

inline void cleanup_config() {
    if (target_assembly != NULL)
        free(target_assembly);
}
