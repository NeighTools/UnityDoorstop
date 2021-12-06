#include "../config.h"
#include "../crt.h"
#include "../logging.h"
#include "../util.h"

#define CONFIG_NAME TEXT("doorstop_config.ini")
#define DEFAULT_TARGET_ASSEMBLY TEXT("Doorstop.dll")
#define EXE_EXTENSION_LENGTH 4
#define STR_EQUAL(str1, str2) (lstrcmpi(str1, str2) == 0)

void load_bool_file(const char_t *path, const char_t *section,
                    const char_t *key, const char_t *def, bool_t *value) {
    char_t enabled_string[256] = TEXT("true");
    GetPrivateProfileString(section, key, def, enabled_string, 256, path);
    LOG("CONFIG: %S.%S = %S\n", section, key, enabled_string);

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

void load_path_file(const char_t *path, const char_t *section,
                    const char_t *key, const char_t *def, char_t **value) {
    char_t *tmp = get_ini_entry(path, section, key, def);
    LOG("CONFIG: %S.%S = %S\n", section, key, tmp);
    if (!tmp)
        return;
    *value = get_full_path(tmp);
    free(tmp);
}

inline void init_config_file() {
    if (!file_exists(CONFIG_NAME))
        return;

    char_t *config_path = get_full_path(CONFIG_NAME);

    load_bool_file(config_path, TEXT("UnityDoorstop"), TEXT("enabled"),
                   TEXT("true"), &config.enabled);
    load_bool_file(config_path, TEXT("UnityDoorstop"),
                   TEXT("ignoreDisableSwitch"), TEXT("false"),
                   &config.ignore_disabled_env);
    load_bool_file(config_path, TEXT("UnityDoorstop"),
                   TEXT("redirectOutputLog"), TEXT("false"),
                   &config.redirect_output_log);
    load_path_file(config_path, TEXT("UnityDoorstop"), TEXT("targetAssembly"),
                   DEFAULT_TARGET_ASSEMBLY, &config.target_assembly);

    load_path_file(config_path, TEXT("MonoBackend"), TEXT("runtimeLib"), NULL,
                   &config.mono_lib_dir);
    load_path_file(config_path, TEXT("MonoBackend"), TEXT("configDir"), NULL,
                   &config.mono_config_dir);
    load_path_file(config_path, TEXT("MonoBackend"), TEXT("corlibDir"), NULL,
                   &config.mono_corlib_dir);

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
        LOG("ARGV: %S = %S\n", arg_name, par);
        return TRUE;
    }
    return FALSE;
}

bool_t load_path_argv(char_t **argv, int *i, int argc, const char_t *arg_name,
                      char_t **value) {
    if (STR_EQUAL(argv[*i], arg_name) && *i < argc) {
        if (*value != NULL)
            free(*value);
        const size_t len = strlen(argv[*i + 1]) + 1;
        *value = malloc(sizeof(char_t) * len);
        strncpy(*value, argv[++*i], len);
        LOG("ARGV: %S = %S\n", arg_name, *value);
        return TRUE;
    }
    return FALSE;
}

inline void init_cmd_args() {
    char_t *args = GetCommandLine();
    int argc = 0;
    char_t **argv = CommandLineToArgv(args, &argc);

#define PARSE_ARG(name, dest, parser)                                          \
    if (parser(argv, &i, argc, name, &(dest)))                                 \
        continue;

    for (int i = 0; i < argc; i++) {
        PARSE_ARG(TEXT("--doorstop-enable"), config.enabled, load_bool_argv);
        PARSE_ARG(TEXT("--redirect-output-log"), config.redirect_output_log,
                  load_bool_argv);
        PARSE_ARG(TEXT("--doorstop-target"), config.target_assembly,
                  load_path_argv);

        PARSE_ARG(TEXT("--mono-runtime-lib"), config.mono_lib_dir,
                  load_path_argv);
        PARSE_ARG(TEXT("--mono-config-dir"), config.mono_config_dir,
                  load_path_argv);
        PARSE_ARG(TEXT("--mono-corlib-dir"), config.mono_config_dir,
                  load_path_argv);
    }

    LocalFree(argv);

#undef PARSE_ARG
}

inline void init_env_vars() {
    char_t *disable_env = getenv(TEXT("DOORSTOP_DISABLE"));
    if (!config.ignore_disabled_env && disable_env != 0) {
        LOG("DOORSTOP_DISABLE is set! Disabling Doorstop!\n");
        config.enabled = FALSE;
    }
    shutenv(disable_env);
}

void load_config() {
    config.enabled = FALSE;
    config.ignore_disabled_env = FALSE;
    config.redirect_output_log = FALSE;
    config.mono_config_dir = NULL;
    config.mono_corlib_dir = NULL;
    config.mono_lib_dir = NULL;
    config.target_assembly = NULL;

    init_config_file();
    init_cmd_args();
    init_env_vars();
}

void cleanup_config() {
#define FREE_NON_NULL(val)                                                     \
    if (val != NULL) {                                                         \
        free(val);                                                             \
        val = NULL;                                                            \
    }

    FREE_NON_NULL(config.target_assembly);
    FREE_NON_NULL(config.mono_lib_dir);
    FREE_NON_NULL(config.mono_config_dir);
    FREE_NON_NULL(config.mono_corlib_dir);

#undef FREE_NON_NULL
}