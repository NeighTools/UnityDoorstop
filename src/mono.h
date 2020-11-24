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
#endif

#ifndef MONO_H
#define MONO_H

#define IMPORT_PREFIX mono
#include "func_import.h"
#undef IMPORT_PREFIX

#endif