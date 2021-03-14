#ifndef CONFIG_H
#define CONFIG_H

#include "util.h"

/**
 * @brief Doorstop configuration
 */
struct {
    /**
     * @brief Whether Doorstop is enabled (enables hooking methods and executing
     * target assembly).
     */
    bool_t enabled;

    /**
     * @brief Whether to redirect Unity output log.
     *
     * If enabled, Doorstop will append `-logFile` command line argument to
     * process command line arguments. This will force Unity to write output
     * logs to the game folder.
     */
    bool_t redirect_output_log;

    /**
     * @brief Whether to ignore DOORSTOP_DISABLE.
     *
     * If enabled, Doorstop will ignore DOORSTOP_DISABLE environment variable.
     * This is sometimes useful with Steam games that break env var isolation.
     */
    bool_t ignore_disabled_env;

    /**
     * @brief Path to a managed assembly to invoke.
     */
    char_t *target_assembly;

    /**
     * @brief Path to mono library when bootstrapping Il2Cpp.
     */
    char_t *mono_lib_dir;

    /**
     * @brief Path to mono config directory when bootstrapping Il2Cpp.
     */
    char_t *mono_config_dir;

    /**
     * @brief Path to mono root directory when bootstrapping Il2Cpp.
     */
    char_t *mono_corlib_dir;
} config;

/**
 * @brief Load configuration.
 */
void load_config();

/**
 * @brief Clean up configuration.
 */
void cleanup_config();
#endif