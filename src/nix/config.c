#include "../config/config.h"
#include "../util/logging.h"
#include "../crt.h"

void get_env_bool(const char_t *name, bool_t *target) {
    char_t *value = getenv(name);
    if (value != NULL && strcmp(value, "1") == 0) {
        *target = TRUE;
    } else if (value == NULL || strcmp(value, "0") == 0) {
        *target = FALSE;
    }
}

void try_get_env(const char_t *name, char_t *def, char_t **target) {
    char_t *value = getenv(name);
    if (value != NULL && strlen(value) > 0) {
        *target = strdup(value);
    } else {
        *target = def;
    }
}

void get_env_path(const char_t *name, char_t **target) {
    char_t *value = getenv(name);
    if (value != NULL && strlen(value) > 0) {
        *target = get_full_path(value);
    } else {
        *target = NULL;
    }
}

void load_config() {
    get_env_bool("DOORSTOP_ENABLED", &config.enabled);
    get_env_bool("DOORSTOP_REDIRECT_OUTPUT_LOG", &config.redirect_output_log);
    get_env_bool("DOORSTOP_IGNORE_DISABLED_ENV", &config.ignore_disabled_env);
    get_env_bool("DOORSTOP_MONO_DEBUG_ENABLED", &config.mono_debug_enabled);
    get_env_bool("DOORSTOP_MONO_DEBUG_SUSPEND", &config.mono_debug_suspend);
    try_get_env("DOORSTOP_MONO_DEBUG_ADDRESS", TEXT("127.0.0.1:10000"),
                &config.mono_debug_address);
    get_env_path("DOORSTOP_TARGET_NATIVE_ASSEMBLY", &config.target_native_library);
    get_env_path("DOORSTOP_TARGET_ASSEMBLY", &config.target_assembly);
    get_env_path("DOORSTOP_BOOT_CONFIG_OVERRIDE", &config.boot_config_override);
    try_get_env("DOORSTOP_MONO_DLL_SEARCH_PATH_OVERRIDE", TEXT(""),
                &config.mono_dll_search_path_override);
    get_env_path("DOORSTOP_CLR_RUNTIME_CORECLR_PATH",
                 &config.clr_runtime_coreclr_path);
    get_env_path("DOORSTOP_CLR_CORLIB_DIR", &config.clr_corlib_dir);

    //Print out all the relevant configuration settings using LOG()
    LOG("DOORSTOP_ENABLED: %d", config.enabled);
    LOG("DOORSTOP_REDIRECT_OUTPUT_LOG: %d", config.redirect_output_log);
    LOG("DOORSTOP_IGNORE_DISABLED_ENV: %d", config.ignore_disabled_env);
    LOG("DOORSTOP_MONO_DEBUG_ENABLED: %d", config.mono_debug_enabled);
    LOG("DOORSTOP_MONO_DEBUG_SUSPEND: %d", config.mono_debug_suspend);
    LOG("DOORSTOP_MONO_DEBUG_ADDRESS: %s", config.mono_debug_address);
    LOG("DOORSTOP_TARGET_NATIVE_ASSEMBLY: %s", config.target_native_library);
    LOG("DOORSTOP_TARGET_ASSEMBLY: %s", config.target_assembly);
    LOG("DOORSTOP_BOOT_CONFIG_OVERRIDE: %s", config.boot_config_override);
    LOG("DOORSTOP_MONO_DLL_SEARCH_PATH_OVERRIDE: %s",
        config.mono_dll_search_path_override);
    LOG("DOORSTOP_CLR_RUNTIME_CORECLR_PATH: %s", config.clr_runtime_coreclr_path);
    LOG("DOORSTOP_CLR_CORLIB_DIR: %s", config.clr_corlib_dir);
}