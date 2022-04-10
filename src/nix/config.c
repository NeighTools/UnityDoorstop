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

void try_get_env(const char_t *name, const char_t *def, char_t **target) {
    char_t *value = getenv(name);
    if (value != NULL && strlen(value) > 0) {
        *target = strdup(value);
    } else {
        *target = def;
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
    try_get_env("DOORSTOP_MONO_DEBUG_ADDRESS", TEXT("127.0.0.1:10000"),
                &config.mono_debug_address);
    try_get_env("DOORSTOP_TARGET_ASSEMBLY", TEXT("Doorstop.dll"),
                &config.target_assembly);
    try_get_env("DOORSTOP_MONO_DLL_SEARCH_PATH_OVERRIDE", NULL,
                &config.mono_dll_search_path_override);
    try_get_env("DOORSTOP_CLR_RUNTIME_CORECLR_PATH", NULL,
                &config.clr_runtime_coreclr_path);
    try_get_env("DOORSTOP_CLR_CORLIB_DIR", NULL, &config.clr_corlib_dir);
}