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
#include <cstdint>

// Define a helper macro that creates a typedef and a variable that will hold address to a mono.dll function
#define DEF_MONO_PROC(name, returnType, ...)          \
	typedef returnType (__cdecl * name##_t)(__VA_ARGS__); \
	static name##_t name

// A helper macro to load the function address from a library
#define GET_MONO_PROC(name, lib) name = reinterpret_cast<name##_t>(GetProcAddress(lib, #name))

// Creates a MonoString based from a C wide string
#define MONO_STRING(str) mono_string_new_utf16(domain, str, wcslen(str))

// Set MonoArray's index to a reference type value (i.e. string)
#define SET_ARRAY_REF(arr, index, refVal) \
	{ \
		const auto p = (void**) mono_array_addr_with_size(arr, sizeof(void*), index); \
		mono_gc_wbarrier_set_arrayref(arr, p, refVal); \
	}

namespace Mono
{
	// MonoAssembly * mono_domain_assembly_open  (MonoDomain *domain, const char *name);
	DEF_MONO_PROC(mono_domain_assembly_open, void *, void *, const char *);

	// MonoImage * mono_assembly_get_image(MonoAssembly *assembly);
	DEF_MONO_PROC(mono_assembly_get_image, void *, void *);

	// MonoObject * mono_runtime_invoke(MonoMethod *method, void *obj, void **params, MonoObject **exc);
	DEF_MONO_PROC(mono_runtime_invoke, void *, void *, void *, void **, void **);

	// MonoDomain * mono_jit_init_version(const char *root_domain_name, const char *runtime_version);
	DEF_MONO_PROC(mono_jit_init_version, void *, const char *, const char *);

	// MonoMethodDesc* mono_method_desc_new(const char *name, gboolean include_namespace)
	DEF_MONO_PROC(mono_method_desc_new, void *, const char *, int);

	// MonoMethod* mono_method_desc_search_in_image (MonoMethodDesc *desc, MonoImage *image)
	DEF_MONO_PROC(mono_method_desc_search_in_image, void *, void *, void *);

	//MonoMethodSignature* mono_method_signature (MonoMethod *m)
	DEF_MONO_PROC(mono_method_signature, void *, void *);

	//guint32 mono_signature_get_param_count (MonoMethodSignature *sig)
	DEF_MONO_PROC(mono_signature_get_param_count, uint32_t, void *);

	//MonoArray* mono_array_new (MonoDomain *domain, MonoClass *eclass, uintptr_t n)
	DEF_MONO_PROC(mono_array_new, void *, void *, void *, uintptr_t);

	//MonoClass* mono_get_string_class (void)
	DEF_MONO_PROC(mono_get_string_class, void *);

	//MonoString* mono_string_new_utf16 (MonoDomain *domain, const guint16 *text, gint32 len)
	DEF_MONO_PROC(mono_string_new_utf16, void *, void *, const wchar_t *, int32_t);

	//MONO_API void mono_gc_wbarrier_set_arrayref  (MonoArray *arr, void* slot_ptr, MonoObject* value);
	DEF_MONO_PROC(mono_gc_wbarrier_set_arrayref, void, void *, void *, void *);

	// MONO_API char* mono_array_addr_with_size(MonoArray *array, int size, uintptr_t idx);
	DEF_MONO_PROC(mono_array_addr_with_size, char *, void *, int, uintptr_t);


	/**
	* \brief Loads Mono C API function pointers so that the above definitions can be called.
	* \param monoLib Mono.dll module.
	*/
	inline void loadMonoFunctions(HMODULE monoLib)
	{
		// Find and assign all our functions that we are going to use
		GET_MONO_PROC(mono_domain_assembly_open, monoLib);
		GET_MONO_PROC(mono_assembly_get_image, monoLib);
		GET_MONO_PROC(mono_runtime_invoke, monoLib);
		GET_MONO_PROC(mono_jit_init_version, monoLib);
		GET_MONO_PROC(mono_method_desc_new, monoLib);
		GET_MONO_PROC(mono_method_desc_search_in_image, monoLib);
		GET_MONO_PROC(mono_method_signature, monoLib);
		GET_MONO_PROC(mono_signature_get_param_count, monoLib);
		GET_MONO_PROC(mono_array_new, monoLib);
		GET_MONO_PROC(mono_get_string_class, monoLib);
		GET_MONO_PROC(mono_string_new_utf16, monoLib);
		GET_MONO_PROC(mono_gc_wbarrier_set_arrayref, monoLib);
		GET_MONO_PROC(mono_array_addr_with_size, monoLib);
	}

	// Our original mono_jit_init_version_original
	static mono_jit_init_version_t mono_jit_init_version_original;

	inline std::wstring getMonoPath(std::wstring prefix = L"\\")
	{
		// Code to get the name of the Game's Executable
		wchar_t path[MAX_PATH];
		wchar_t name[_MAX_FNAME];

		GetModuleFileName(nullptr, path, sizeof(path));
		_wsplitpath_s(path, nullptr, 0, nullptr, 0, name, sizeof(name), nullptr, 0);

		// The mono.dll should *usually* be in GameName_Data\Mono
		// TODO: A better way to find mono.dll?
		return std::wstring(L".\\").append(name).append(L"_Data\\Mono").append(prefix).append(L"mono.dll");
	}
}
