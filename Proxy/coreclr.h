#pragma once
#include <wtypes.h>

int (*coreclr_initialize)(
    const char* exePath,
    const char* appDomainFriendlyName,
    int propertyCount,
    const char** propertyKeys,
    const char** propertyValues,
    void** hostHandle,
    unsigned int* domainId);

int (*coreclr_create_delegate)(
    void* hostHandle,
    unsigned int domainId,
    const char* entryPointAssemblyName,
    const char* entryPointTypeName,
    const char* entryPointMethodName,
    void** delegate);

inline void load_coreclr_functions(HMODULE coreclr_lib) {
#define GET_CORECLR_PROC(name) name = (void*)GetProcAddress(coreclr_lib, #name)

    GET_CORECLR_PROC(coreclr_initialize);
    GET_CORECLR_PROC(coreclr_create_delegate);

#undef GET_CORECLR_PROC
}