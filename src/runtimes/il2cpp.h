#ifdef DEFINE_CALLS
DEF_CALL(int, init, const char *domain_name)
DEF_CALL(void *, runtime_invoke, void *method, void *obj, void **params,
         void **exec)
DEF_CALL(const char *, method_get_name, void *method)
#else

#ifndef IL2CPP_H
#define IL2CPP_H

#define IMPORT_PREFIX il2cpp
#if _WIN32
#define IMPORT_CONV __cdecl
#else
#define IMPORT_CONV __attribute__((cdecl))
#endif
#include "func_import.h"
#undef IMPORT_PREFIX
#undef IMPORT_CONV

#endif
#endif