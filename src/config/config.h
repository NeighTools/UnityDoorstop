#ifndef CONFIG_H
#define CONFIG_H

#include "../util/util.h"

/**
 * @brief Doorstop configuration
 */
typedef struct {
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
     * @brief Array of paths to managed assemblies to invoke.
     *
     * Populated by parse_target_assembly_string() from a semicolon-delimited
     * target_assembly value in doorstop_config.ini. Supports plain file paths
     * and directory paths (all .dll files in the directory are added).
     */
    char_t **target_assemblies;

    /**
     * @brief Number of assemblies in target_assemblies.
     */
    size_t num_assemblies;

    /**
     * @brief Index of the assembly currently being bootstrapped.
     *
     * Used by bootstrap.c to communicate which assembly is being loaded.
     */
    size_t assembly_index;

    /**
     * @brief Path to a custom boot.config file to use. If enabled, this file
     * takes precedence over the default one in the Data folder.
     */
    char_t *boot_config_override;

    /**
     * @brief Path to use as the main DLL search path. If enabled, this folder
     * takes precedence over the default Managed folder.
     */
    char_t *mono_dll_search_path_override;

    /**
     * @brief Whether to enable the mono debugger.
     */
    bool_t mono_debug_enabled;

    /**
     * @brief Whether to enable the debugger in suspended state.
     *
     * If enabled, the runtime will force the game to wait until a debugger is
     * connected.
     */
    bool_t mono_debug_suspend;

    /**
     * @brief Debug address to use for the mono debugger.
     */
    char_t *mono_debug_address;

    /**
     * @brief Path to the CoreCLR runtime library.
     */
    char_t *clr_runtime_coreclr_path;

    /**
     * @brief Path to the CoreCLR core libraries folder.
     */
    char_t *clr_corlib_dir;
} Config;

extern Config config;

/**
 * @brief Load configuration.
 */
extern void load_config();

/**
 * @brief Initialize default values for configuration.
 */
extern void init_config_defaults();

/**
 * @brief Clean up configuration.
 */
extern void cleanup_config();

/**
 * @brief Parse the target_assembly config string into config.target_assemblies.
 *
 * Supports semicolon-separated paths. Each entry may be:
 *   - A .dll file path (absolute or relative to CWD)
 *   - A directory path — all *.dll files in that directory are added
 *
 * Populates config.target_assemblies and config.num_assemblies.
 */
void parse_target_assembly_string(char_t *target_assembly);
#endif