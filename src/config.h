#ifndef CONFIG_H
#define CONFIG_H

#include "util.h"

struct {
    bool_t enabled;
    bool_t redirect_output_log;
    bool_t ignore_disabled_env;
    char_t *target_assembly;
    char_t *mono_lib_dir;
    char_t *mono_config_dir;
    char_t *mono_corlib_dir;
} config;

void load_config();
void cleanup_config();
#endif