#include "config.h"
#include "crt.h"
#include "assert_util.h"
#include "winapi_util.h"

#define STR_EQUAL(str1, str2) (lstrcmpiW(str1, str2) == 0)

void load_bool_file(const wchar_t *path, const wchar_t *section, const wchar_t *key, const wchar_t *def, BOOL *value) {
    wchar_t enabled_string[256] = L"true";
    GetPrivateProfileStringW(section, key, def, enabled_string, 256, path);
    LOG("CONFIG: %S.%S = %S\n", section, key, enabled_string);

    if (STR_EQUAL(enabled_string, L"true"))
        *value = TRUE;
    else if (STR_EQUAL(enabled_string, L"false"))
        *value = FALSE;
}

void load_path_file(const wchar_t *path, const wchar_t *section, const wchar_t *key, const wchar_t *def,
                    wchar_t **value) {
    wchar_t *tmp = get_ini_entry(path, section, key, def);
    LOG("CONFIG: %S.%S = %S\n", section, key, tmp);
    if (!tmp || wcslen(tmp) == 0)
        return;
    *value = get_full_path(tmp);
    free(tmp);
}

inline void init_config_file() {
    if (!file_exists(CONFIG_NAME))
        return;

    wchar_t *config_path = get_full_path(CONFIG_NAME);

    load_bool_file(config_path, L"UnityDoorstop", L"enabled", L"true", &config.enabled);
    load_bool_file(config_path, L"UnityDoorstop", L"ignoreDisableSwitch", L"false", &config.ignore_disabled_env);
    load_bool_file(config_path, L"UnityDoorstop", L"redirectOutputLog", L"false", &config.redirect_output_log);
    load_path_file(config_path, L"UnityDoorstop", L"targetAssembly", DEFAULT_TARGET_ASSEMBLY, &config.target_assembly);
    load_path_file(config_path, L"UnityDoorstop", L"dllSearchPathOverride", NULL, &config.mono_dll_search_path_override);

    load_path_file(config_path, L"MonoBackend", L"runtimeLib", NULL, &config.mono_lib_dir);
    load_path_file(config_path, L"MonoBackend", L"configDir", NULL, &config.mono_config_dir);
    load_path_file(config_path, L"MonoBackend", L"corlibDir", NULL, &config.mono_corlib_dir);

    free(config_path);
}

BOOL load_bool_argv(wchar_t **argv, int *i, int argc, const wchar_t *arg_name, BOOL *value) {
    if (STR_EQUAL(argv[*i], arg_name) && *i < argc) {
        wchar_t *par = argv[++*i];
        if (STR_EQUAL(par, L"true"))
            *value = TRUE;
        else if (STR_EQUAL(par, L"false"))
            *value = FALSE;
        LOG("ARGV: %S = %S\n", arg_name, par);
        return TRUE;
    }
    return FALSE;
}

BOOL load_path_argv(wchar_t **argv, int *i, int argc, const wchar_t *arg_name, wchar_t **value) {
    if (STR_EQUAL(argv[*i], arg_name) && *i < argc) {
        if (*value != NULL)
            free(*value);
        const size_t len = wcslen(argv[*i + 1]) + 1;
        *value = malloc(sizeof(wchar_t) * len);
        wmemcpy(*value, argv[++*i], len);
        LOG("ARGV: %S = %S\n", arg_name, *value);
        return TRUE;
    }
    return FALSE;
}

inline void init_cmd_args() {
    wchar_t *args = GetCommandLineW();
    int argc = 0;
    wchar_t **argv = CommandLineToArgvW(args, &argc);

#define PARSE_ARG(name, dest, parser)           \
    if (parser(argv, &i, argc, name, &(dest)))  \
        continue;

    for (int i = 0; i < argc; i++) {
        PARSE_ARG(L"--doorstop-enable", config.enabled, load_bool_argv);
        PARSE_ARG(L"--redirect-output-log", config.redirect_output_log, load_bool_argv);
        PARSE_ARG(L"--doorstop-target", config.target_assembly, load_path_argv);

        PARSE_ARG(L"--mono-runtime-lib", config.mono_lib_dir, load_path_argv);
        PARSE_ARG(L"--mono-config-dir", config.mono_config_dir, load_path_argv);
        PARSE_ARG(L"--mono-corlib-dir", config.mono_config_dir, load_path_argv);
    }

    LocalFree(argv);

#undef PARSE_ARG
}

inline void init_env_vars() {
    if (!config.ignore_disabled_env && GetEnvironmentVariableW(L"DOORSTOP_DISABLE", NULL, 0) != 0) {
        LOG("DOORSTOP_DISABLE is set! Disabling Doorstop!\n");
        config.enabled = FALSE;
    }
}

void load_config() {
    config.enabled = FALSE;
    config.ignore_disabled_env = FALSE;
    config.redirect_output_log = FALSE;
    config.mono_config_dir = NULL;
    config.mono_corlib_dir = NULL;
    config.mono_lib_dir = NULL;
    config.target_assembly = NULL;
    config.mono_dll_search_path_override = NULL;

    init_config_file();
    init_cmd_args();
    init_env_vars();
}
