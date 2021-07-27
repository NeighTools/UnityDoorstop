#ifndef MAIN_H
#define MAIN_H

#include "util.h"

void *init_mono(const char *root_domain_name, const char *runtime_version);
int init_il2cpp(const char *domain_name);

void start_logger(void *doorstop_module, bool_t fixed_cwd);

#endif