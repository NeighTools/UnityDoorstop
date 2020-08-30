/**
 * il2cpp.h -- Definitions for IL2CPP runtime functions.
 *
 * This is like mono.h but for IL2CPP runtime functions.
 * Generally, IL2CPP provides similar API as mono with some
 * exceptions.
 */

#pragma once

#include <windows.h>

int (*il2cpp_init)(const char* domain_name);
void* (*il2cpp_runtime_invoke)(void *method, void *obj, void **params, void **exc);
const char* (*il2cpp_method_get_name)(void *method);

inline void load_il2cpp_functions(HMODULE game_assembly) {
#define GET_IL2CPP_PROC(name) name = (void*)GetProcAddress(game_assembly, #name)

	GET_IL2CPP_PROC(il2cpp_init);
	GET_IL2CPP_PROC(il2cpp_runtime_invoke);
	GET_IL2CPP_PROC(il2cpp_method_get_name);

#undef GET_IL2CPP_PROC
}