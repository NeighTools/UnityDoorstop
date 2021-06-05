/*
 * mono.h -- Definitions for Mono C API functions.
 * 
 * The file contains definitions for some of functions provided by Mono C API.
 * 
 * Note: Since we don't use any mono headers, all pointers to mono-related structs are
 * replaced with void *.
 */

#pragma once

#include <windows.h>

// Here we define the pointers to some functions within mono.dll
// Note to C learners: these are not signature definitions, but rather "variable"
// definitions with the function pointer type.

// Note: we use void* instead of the real intented structs defined in mono API
// This way we don't need to include or define any of Mono's structs, which saves space
// This, obviously, comes with a drawback of not being able to easily access the contents of the structs

void * (*mono_thread_current)();
void (*mono_thread_set_main)(void *);

void *(*mono_jit_init_version)(const char *root_domain_name, const char *runtime_version);
void *(*mono_domain_assembly_open)(void *domain, const char *name);
void *(*mono_assembly_get_image)(void *assembly);
void *(*mono_runtime_invoke)(void *method, void *obj, void **params, void **exc);

void *(*mono_method_desc_new)(const char *name, int include_namespace);
void* (*mono_method_desc_search_in_image)(void* desc, void* image);
void *(*mono_method_desc_search_in_class)(void *desc, void *klass);
void (*mono_method_desc_free)(void *desc);
void *(*mono_method_signature)(void *method);
UINT32 (*mono_signature_get_param_count)(void *sig);

void (*mono_domain_set_config)(void *domain, char *base_dir, char *config_file_name);
void *(*mono_array_new)(void *domain, void *eclass, uintptr_t n);
void *(*mono_get_string_class)();

char *(*mono_assembly_getrootdir)();

// Additional funcs to bootstrap custom MONO
void (*mono_set_dirs)(const char* assembly_dir, const char* config_dir);
void (*mono_config_parse)(const char* filename);
void (*mono_set_assemblies_path)(const char* path);
void *(*mono_object_to_string)(void* obj, void** exc);
char *(*mono_string_to_utf8)(void* s);

void *(*mono_image_open_from_data_with_name)(void *data, DWORD data_len, int need_copy, void *status, int refonly,
                                             const char *name);

void* (*mono_get_exception_class)();
void* (*mono_object_get_virtual_method)(void* obj_raw, void* method);

void* (*mono_jit_parse_options)(int argc, const char *argv);

typedef enum {
    MONO_DEBUG_FORMAT_NONE,
    MONO_DEBUG_FORMAT_MONO,
    /* Deprecated, the mdb debugger is not longer supported. */
    MONO_DEBUG_FORMAT_DEBUGGER
} MonoDebugFormat;

void* (*mono_debug_init)(MonoDebugFormat format);
void* (*mono_debug_domain_create)(void *domain);

/**
* \brief Loads Mono C API function pointers so that the above definitions can be called.
* \param mono_lib Mono.dll module.
*/
inline void load_mono_functions(HMODULE mono_lib) {
    // Enjoy the fact that C allows such sloppy casting
    // In C++ you would have to cast to the precise function pointer type
#define GET_MONO_PROC(name) name = (void*)GetProcAddress(mono_lib, #name)

    // Find and assign all our functions that we are going to use
    GET_MONO_PROC(mono_domain_assembly_open);
    GET_MONO_PROC(mono_assembly_get_image);
    GET_MONO_PROC(mono_runtime_invoke);
    GET_MONO_PROC(mono_jit_init_version);
    GET_MONO_PROC(mono_method_desc_new);
    GET_MONO_PROC(mono_method_desc_search_in_class);
    GET_MONO_PROC(mono_method_desc_search_in_image);
    GET_MONO_PROC(mono_method_desc_free);
    GET_MONO_PROC(mono_method_signature);
    GET_MONO_PROC(mono_signature_get_param_count);
    GET_MONO_PROC(mono_array_new);
    GET_MONO_PROC(mono_get_string_class);
    GET_MONO_PROC(mono_assembly_getrootdir);
    GET_MONO_PROC(mono_thread_current);
    GET_MONO_PROC(mono_thread_set_main);
    GET_MONO_PROC(mono_domain_set_config);
    GET_MONO_PROC(mono_set_dirs);
    GET_MONO_PROC(mono_config_parse);
    GET_MONO_PROC(mono_set_assemblies_path);
    GET_MONO_PROC(mono_object_to_string);
    GET_MONO_PROC(mono_string_to_utf8);
    GET_MONO_PROC(mono_image_open_from_data_with_name);
    GET_MONO_PROC(mono_get_exception_class);
    GET_MONO_PROC(mono_object_get_virtual_method);
    GET_MONO_PROC(mono_jit_parse_options);
    GET_MONO_PROC(mono_debug_init);
    GET_MONO_PROC(mono_debug_domain_create);

#undef GET_MONO_PROC
}
