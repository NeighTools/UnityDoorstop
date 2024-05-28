#include "../config/config.h"
#include "../crt.h"
#include "../util/logging.h"
#include "../util/util.h"

#define CONFIG_NAME TEXT("doorstop_config.ini")
#define DEFAULT_TARGET_ASSEMBLY TEXT("Doorstop.dll")
#define EXE_EXTENSION_LENGTH 4
#define STR_EQUAL(str1, str2) (lstrcmpi(str1, str2) == 0)

void load_bool_file(const char_t *path, const char_t *section,
                    const char_t *key, const char_t *def, bool_t *value) {
    char_t enabled_string[256] = TEXT("true");
    GetPrivateProfileString(section, key, def, enabled_string, 256, path);
    LOG("CONFIG: %s.%s = %s", section, key, enabled_string);

    if (STR_EQUAL(enabled_string, TEXT("true")))
        *value = TRUE;
    else if (STR_EQUAL(enabled_string, TEXT("false")))
        *value = FALSE;
}

char_t *get_ini_entry(const char_t *config_file, const char_t *section,
                      const char_t *key, const char_t *default_val) {
    DWORD i = 0;
    DWORD size, read;
    char_t *result = NULL;
    do {
        if (result != NULL)
            free(result);
        i++;
        size = i * MAX_PATH + 1;
        result = malloc(sizeof(char_t) * size);
        read = GetPrivateProfileString(section, key, default_val, result, size,
                                       config_file);
    } while (read == size - 1);
    return result;
}

bool_t load_str_file(const char_t *path, const char_t *section,
                     const char_t *key, const char_t *def, char_t **value) {
    char_t *tmp = get_ini_entry(path, section, key, def);
    LOG("CONFIG: %s.%s = %s", section, key, tmp);
    if (!tmp || strlen(tmp) == 0)
        return FALSE;
    *value = tmp;
    return TRUE;
}

void load_path_file(const char_t *path, const char_t *section,
                    const char_t *key, const char_t *def, char_t **value) {
    if (!load_str_file(path, section, key, def, value))
        return;
    char_t *tmp = *value;
    *value = get_full_path(tmp);
    LOG("(%s.%s) %s => %s", section, key, tmp, *value);
    free(tmp);
}

static inline void init_config_file() {
    if (!file_exists(CONFIG_NAME))
        return;

    char_t *config_path = get_full_path(CONFIG_NAME);

    load_bool_file(config_path, TEXT("General"), TEXT("enabled"), TEXT("true"),
                   &config.enabled);
    load_bool_file(config_path, TEXT("General"), TEXT("ignore_disable_switch"),
                   TEXT("false"), &config.ignore_disabled_env);
    load_bool_file(config_path, TEXT("General"), TEXT("redirect_output_log"),
                   TEXT("false"), &config.redirect_output_log);
    load_path_file(config_path, TEXT("General"), TEXT("target_assembly"),
                   DEFAULT_TARGET_ASSEMBLY, &config.target_assembly);
    load_path_file(config_path, TEXT("General"), TEXT("boot_config_override"),
                   NULL, &config.boot_config_override);

    load_str_file(config_path, TEXT("UnityMono"),
                  TEXT("dll_search_path_override"), TEXT(""),
                  &config.mono_dll_search_path_override);
    load_bool_file(config_path, TEXT("UnityMono"), TEXT("debug_enabled"),
                   TEXT("false"), &config.mono_debug_enabled);
    load_bool_file(config_path, TEXT("UnityMono"), TEXT("debug_suspend"),
                   TEXT("false"), &config.mono_debug_suspend);
    load_str_file(config_path, TEXT("UnityMono"), TEXT("debug_address"),
                  TEXT("127.0.0.1:10000"), &config.mono_debug_address);

    load_path_file(config_path, TEXT("Il2Cpp"), TEXT("coreclr_path"), NULL,
                   &config.clr_runtime_coreclr_path);
    load_path_file(config_path, TEXT("Il2Cpp"), TEXT("corlib_dir"), NULL,
                   &config.clr_corlib_dir);

    free(config_path);
}

bool_t load_bool_argv(char_t **argv, int *i, int argc, const char_t *arg_name,
                      bool_t *value) {
    if (STR_EQUAL(argv[*i], arg_name) && *i < argc) {
        char_t *par = argv[++*i];
        if (STR_EQUAL(par, TEXT("true")))
            *value = TRUE;
        else if (STR_EQUAL(par, TEXT("false")))
            *value = FALSE;
        LOG("ARGV: %s = %s", arg_name, par);
        return TRUE;
    }
    return FALSE;
}

bool_t load_str_argv(char_t **argv, int *i, int argc, const char_t *arg_name,
                     char_t **value) {
    if (STR_EQUAL(argv[*i], arg_name) && *i < argc) {
        if (*value != NULL)
            free(*value);
        const size_t len = strlen(argv[*i + 1]) + 1;
        *value = malloc(sizeof(char_t) * len);
        strncpy(*value, argv[++*i], len);
        LOG("ARGV: %s = %s", arg_name, *value);
        return TRUE;
    }
    return FALSE;
}

bool_t load_path_argv(char_t **argv, int *i, int argc, const char_t *arg_name,
                      char_t **value) {
    if (!load_str_argv(argv, i, argc, arg_name, value))
        return FALSE;
    char_t *tmp = *value;
    *value = get_full_path(tmp);
    LOG("(%s) %s => %s", arg_name, tmp, *value);
    free(tmp);
    return TRUE;
}

static inline void init_cmd_args() {
    char_t *args = GetCommandLine();
    int argc = 0;
    char_t **argv = CommandLineToArgv(args, &argc);

#define PARSE_ARG(name, dest, parser)                                          \
    if (parser(argv, &i, argc, name, &(dest)))                                 \
        continue;

    for (int i = 0; i < argc; i++) {
        PARSE_ARG(TEXT("--doorstop-enabled"), config.enabled, load_bool_argv);
        PARSE_ARG(TEXT("--doorstop-redirect-output-log"),
                  config.redirect_output_log, load_bool_argv);
        PARSE_ARG(TEXT("--doorstop-target-assembly"), config.target_assembly,
                  load_path_argv);
        PARSE_ARG(TEXT("--doorstop-boot-config-override"),
                  config.boot_config_override, load_path_argv);

        PARSE_ARG(TEXT("--doorstop-mono-dll-search-path-override"),
                  config.mono_dll_search_path_override, load_path_argv);
        PARSE_ARG(TEXT("--doorstop-mono-debug-enabled"),
                  config.mono_debug_enabled, load_bool_argv);
        PARSE_ARG(TEXT("--doorstop-mono-debug-suspend"),
                  config.mono_debug_suspend, load_bool_argv);
        PARSE_ARG(TEXT("--doorstop-mono-debug-address"),
                  config.mono_debug_address, load_str_argv);

        PARSE_ARG(TEXT("--doorstop-clr-corlib-dir"), config.clr_corlib_dir,
                  load_path_argv);
        PARSE_ARG(TEXT("--doorstop-clr-runtime-coreclr-path"),
                  config.clr_runtime_coreclr_path, load_path_argv);
    }

    LocalFree(argv);

#undef PARSE_ARG
}

static inline void init_env_vars() {
    char_t *disable_env = getenv(TEXT("DOORSTOP_DISABLE"));
    if (!config.ignore_disabled_env && disable_env != 0) {
        LOG("DOORSTOP_DISABLE is set! Disabling Doorstop!");
        config.enabled = FALSE;
    }
    shutenv(disable_env);
}

void load_config() {
    init_config_defaults();
    init_config_file();
    init_cmd_args();
    init_env_vars();
}
