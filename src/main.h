#ifndef MAIN_H
#define MAIN_H

#include "il2cpp.h"
#include "mono.h"
#include "util.h"

void *init_mono(const char *root_domain_name, const char *runtime_version);
int init_il2cpp(const char *domain_name);
void *hook_mono_image_open_from_data_with_name(void *data,
                                               unsigned long data_len,
                                               int need_copy,
                                               MonoImageOpenStatus *status,
                                               int refonly, const char *name);
typedef struct {
    char_t *app_path;
    char_t *app_dir;
    char_t *working_dir;
    char_t *doorstop_path;
    char_t *doorstop_filename;
} DoorstopPaths;

DoorstopPaths *paths_init(void *doorstop_module, bool_t fixed_cwd);
void paths_free(DoorstopPaths *paths);

#endif