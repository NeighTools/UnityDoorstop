#ifndef MONO_H
#define MONO_H

void *get_mono_proc(void *lib, const char *name);

#define DEF_MONO_CALL(retType, name, ...)                                      \
    typedef retType (*name##_t)(__VA_ARGS__);
#include "mono_defs.h"
#undef DEF_MONO_CALL

struct {
#define DEF_MONO_CALL(retType, name, ...) name##_t name;
#include "mono_defs.h"
#undef DEF_MONO_CALL
} mono;

inline void load_mono_functions(void *mono_lib) {
#define DEF_MONO_CALL(retType, name, ...)                                      \
    mono.name = (name##_t)get_mono_proc(mono_lib, "mono_" #name);
#include "mono_defs.h"
#undef DEF_MONO_CALL
}

#endif