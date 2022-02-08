#include "../config/config.h"
#include "../crt.h"

void get_env_bool(const char_t *name, bool_t *target) {
    char_t *value = getenv(name);
    if (strcmp(value, "1") == 0) {
        *target = TRUE;
    } else if (strcmp(value, "0") == 0) {
        *target = FALSE;
    }
}

void try_get_env(const char_t *name, char_t **target) {
    char_t *value = getenv(name);
    if (value != NULL) {
        *target = strdup(value);
    }
}

void load_config() {
    get_env_bool("DOORSTOP_ENABLED", &config.enabled);
    get_env_bool("DOORSTOP_REDIRECT_OUTPUT_LOG", &config.redirect_output_log);
    get_env_bool("DOORSTOP_IGNORE_DISABLED_ENV", &config.ignore_disabled_env);
    try_get_env("DOORSTOP_TARGET_ASSEMBLY", &config.target_assembly);
    try_get_env("DOORSTOP_MONO_DLL_SEARCH_PATH_OVERRIDE",
                &config.mono_dll_search_path_override);
    try_get_env("DOORSTOP_CLR_RUNTIME_CORECLR_PATH",
                &config.clr_runtime_coreclr_path);
    try_get_env("DOORSTOP_CLR_CORLIB_DIR", &config.clr_corlib_dir);
}