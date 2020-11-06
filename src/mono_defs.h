// Here we define the pointers to some functions within mono.dll
// NOTE TO DEVELOPERS: you should remove the mono_ prefix when adding new
// mono methods. The prefix is automatically appended when resolving the
// functions.

// Note: we use void* instead of the real intented structs defined in mono API
// This way we don't need to include or define any of Mono's structs, which
// saves space This, obviously, comes with a drawback of not being able to
// easily access the contents of the structs

DEF_MONO_CALL(void *, thread_current);
DEF_MONO_CALL(void, thread_set_main, void *thread);

DEF_MONO_CALL(void *, jit_init_version, const char *root_domain_name,
              const char *runtime_version);
DEF_MONO_CALL(void *, domain_assembly_open, void *domain, const char *name);
DEF_MONO_CALL(void *, assembly_get_image, void *assembly);
DEF_MONO_CALL(void *, runtime_invoke, void *method, void *obj, void **params,
              void **exc);

DEF_MONO_CALL(void *, method_desc_new, const char *name, int include_namespace);
DEF_MONO_CALL(void *, method_desc_search_in_image, void *desc, void *image);
DEF_MONO_CALL(void, method_desc_free, void *desc);
DEF_MONO_CALL(void *, method_signature, void *method);
DEF_MONO_CALL(unsigned int, signature_get_param_count, void *sig);

DEF_MONO_CALL(void, domain_set_config, void *domain, char *base_dir,
              char *config_file_name);
DEF_MONO_CALL(void *, array_new, void *domain, void *eclass, unsigned int n);
DEF_MONO_CALL(void *, get_string_class);

DEF_MONO_CALL(char *, assembly_getrootdir);

DEF_MONO_CALL(void, set_dirs, const char *assembly_dir, const char *config_dir);
DEF_MONO_CALL(void, config_parse, const char *filename);
DEF_MONO_CALL(void, set_assemblies_path, const char *path);
DEF_MONO_CALL(void *, object_to_string, void *obj, void **exc);
DEF_MONO_CALL(char *, string_to_utf8, void *s);
