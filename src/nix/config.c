#include "../config/config.h"
#include "../crt.h"

void get_env_bool(const char_t *name, bool_t *target) {
    char_t *value = getenv(name);
    if (value != NULL && strcmp(value, "1") == 0) {
        *target = TRUE;
    } else if (value == NULL || strcmp(value, "0") == 0) {
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
    get_env_bool("DOORSTOP_MONO_DEBUG_ENABLED", &config.mono_debug_enabled);
    get_env_bool("DOORSTOP_MONO_DEBUG_START_SERVER",
                 &config.mono_debug_start_server);
    get_env_bool("DOORSTOP_MONO_DEBUG_SUSPEND", &config.mono_debug_suspend);
    try_get_env("DOORSTOP_MONO_DEBUG_ADDRESS", &config.mono_debug_address);
    try_get_env("DOORSTOP_TARGET_ASSEMBLY", &config.target_assembly);
    try_get_env("DOORSTOP_MONO_DLL_SEARCH_PATH_OVERRIDE",
                &config.mono_dll_search_path_override);
    try_get_env("DOORSTOP_CLR_RUNTIME_CORECLR_PATH",
                &config.clr_runtime_coreclr_path);
    try_get_env("DOORSTOP_CLR_CORLIB_DIR", &config.clr_corlib_dir);
}