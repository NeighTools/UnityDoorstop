#include "../crt.h"
#include "config.h"

Config config;

void cleanup_config() {
#define FREE_NON_NULL(val)                                                     \
    if (val != NULL) {                                                         \
        free(val);                                                             \
        val = NULL;                                                            \
    }

    FREE_NON_NULL(config.target_assembly);
    FREE_NON_NULL(config.boot_config_override);
    FREE_NON_NULL(config.mono_dll_search_path_override);
    FREE_NON_NULL(config.clr_corlib_dir);
    FREE_NON_NULL(config.clr_runtime_coreclr_path);
    FREE_NON_NULL(config.mono_debug_address);

#undef FREE_NON_NULL
}

void init_config_defaults() {
    config.enabled = FALSE;
    config.ignore_disabled_env = FALSE;
    config.redirect_output_log = FALSE;
    config.mono_debug_enabled = FALSE;
    config.mono_debug_suspend = FALSE;
    config.mono_debug_address = NULL;
    config.target_assembly = NULL;
    config.boot_config_override = NULL;
    config.mono_dll_search_path_override = NULL;
    config.clr_corlib_dir = NULL;
    config.clr_runtime_coreclr_path = NULL;
}