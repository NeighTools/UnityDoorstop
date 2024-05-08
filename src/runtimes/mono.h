#ifdef DEFINE_CALLS
DEF_CALL(void *, thread_current)
DEF_CALL(void, thread_set_main, void *thread)

DEF_CALL(void *, jit_init_version, const char *root_domain_name,
         const char *runtime_version)
DEF_CALL(void *, domain_assembly_open, void *domain, const char *name)
DEF_CALL(void *, assembly_get_image, void *assembly)
DEF_CALL(void *, runtime_invoke, void *method, void *obj, void **params,
         void **exc)

DEF_CALL(void *, method_desc_new, const char *name, int include_namespace)
DEF_CALL(void *, method_desc_search_in_image, void *desc, void *image)
DEF_CALL(void, method_desc_free, void *desc)
DEF_CALL(void *, method_signature, void *method)
DEF_CALL(unsigned int, signature_get_param_count, void *sig)

DEF_CALL(void, domain_set_config, void *domain, char *base_dir,
         char *config_file_name)
DEF_CALL(void *, array_new, void *domain, void *eclass, unsigned int n)
DEF_CALL(void *, get_string_class)

DEF_CALL(char *, assembly_getrootdir)

DEF_CALL(void, set_dirs, const char *assembly_dir, const char *config_dir)
DEF_CALL(void, config_parse, const char *filename)
DEF_CALL(void, set_assemblies_path, const char *path)
DEF_CALL(void *, object_to_string, void *obj, void **exc)
DEF_CALL(char *, string_to_utf8, void *s)
DEF_CALL(void, free, void *ptr)
DEF_CALL(void *, image_open_from_data_with_name, void *data,
         unsigned long data_len, int need_copy, MonoImageOpenStatus *status,
         int refonly, const char *name)
DEF_CALL(void *, assembly_load_from_full, void *image, const char *fname,
         MonoImageOpenStatus *status, int refonly)

DEF_CALL(void *, jit_parse_options, int argc, char **argv)
DEF_CALL(void *, debug_init, MonoDebugFormat format)
DEF_CALL(void *, debug_domain_create, void *domain)
DEF_CALL(int, debug_enabled)
#else

#ifndef MONO_H
#define MONO_H

typedef enum {
    MONO_IMAGE_OK,
    MONO_IMAGE_ERROR_ERRNO,
    MONO_IMAGE_MISSING_ASSEMBLYREF,
    MONO_IMAGE_IMAGE_INVALID
} MonoImageOpenStatus;

typedef enum {
    MONO_DEBUG_FORMAT_NONE,
    MONO_DEBUG_FORMAT_MONO,
    /* Deprecated, the mdb debugger is not longer supported. */
    MONO_DEBUG_FORMAT_DEBUGGER
} MonoDebugFormat;

#define IMPORT_PREFIX mono
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