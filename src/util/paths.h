#ifndef PATHS_H
#define PATHS_H

#include "util.h"

typedef struct {
    char_t *app_path;
    char_t *app_dir;
    char_t *working_dir;
    char_t *doorstop_path;
    char_t *doorstop_filename;
} DoorstopPaths;

DoorstopPaths *paths_init(void *doorstop_module, bool_t fixed_cwd);
void paths_free(DoorstopPaths *const paths);

#endif