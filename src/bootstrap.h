#ifndef BOOTSTRAP_H
#define BOOTSTRAP_H

#include "runtimes/il2cpp.h"
#include "runtimes/mono.h"
#include "util/util.h"

void *init_mono(const char *root_domain_name, const char *runtime_version);
int init_il2cpp(const char *domain_name);
void *hook_mono_image_open_from_data_with_name(void *data,
                                               unsigned long data_len,
                                               int need_copy,
                                               MonoImageOpenStatus *status,
                                               int refonly, const char *name);

#endif