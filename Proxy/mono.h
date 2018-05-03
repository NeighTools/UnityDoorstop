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


namespace Mono
{
	// MonoAssembly * mono_domain_assembly_open  (MonoDomain *domain, const char *name);
	DEF_MONO_PROC(mono_domain_assembly_open, void *, void *, const char *);

	// MonoImage * mono_assembly_get_image(MonoAssembly *assembly);
	DEF_MONO_PROC(mono_assembly_get_image, void *, void *);

	// MonoObject * mono_runtime_invoke(MonoMethod *method, void *obj, void **params, MonoObject **exc);
	DEF_MONO_PROC(mono_runtime_invoke, void *, void *, void *, void **, void **);

	// MonoClass * mono_class_from_name(MonoImage *image, const char* name_space, const char *name);
	DEF_MONO_PROC(mono_class_from_name, void *, void *, const char *, const char *);

	// MonoMethod* mono_class_get_method_from_name (MonoClass *klass, const char *name, int param_count)
	DEF_MONO_PROC(mono_class_get_method_from_name, void *, void *, const char *, int);

	// MonoDomain * mono_jit_init_version(const char *root_domain_name, const char *runtime_version);
	DEF_MONO_PROC(mono_jit_init_version, void *, const char *, const char *);


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
		GET_MONO_PROC(mono_class_from_name, monoLib);
		GET_MONO_PROC(mono_class_get_method_from_name, monoLib);
		GET_MONO_PROC(mono_jit_init_version, monoLib);
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
