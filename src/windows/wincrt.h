/*
 * Custom implementation for common C runtime functions
 * This makes the DLL essentially freestanding on Windows without having to rely
 * on msvcrt.dll
 */
#ifndef WIN_CRT_H
#define WIN_CRT_H

#include "../util.h"
#include <windows.h>

HANDLE h_heap;

#define STR_LEN(str) (sizeof(str) / sizeof((str)[0]))

void *memset(void *dst, int c, size_t n);
#pragma intrinsic(memset)

void *memcpy(void *dst, const void *src, size_t n);
#pragma intrinsic(memcpy)

size_t strlen(char_t const *str);
#pragma intrinsic(strlen)

void *malloc(size_t size);

void *calloc(size_t num, size_t size);

char_t *strcat(char_t *dst, char_t *src);

char_t *strcpy(char_t *dst, char_t *src);

char_t *strncpy(char_t *dst, char_t *src, size_t len);

char_t *dirname(char_t *path);

inline void *dlsym(void *handle, const char *name) {
    return GetProcAddress((HMODULE)handle, name);
}

#define RTLD_LAZY 0x00001

inline void *dlopen(const char_t *filename, int flag) {
    return LoadLibrary(filename);
}

inline void free(void *mem) { HeapFree(h_heap, 0, mem); }

inline int setenv(const char_t *name, const char_t *value, int overwrite) {
    return !SetEnvironmentVariable(name, value);
}

inline char_t *getenv(const char_t *name) {
    DWORD size = GetEnvironmentVariable(name, NULL, 0);
    if (size == 0)
        return NULL;
    char_t *buf = calloc(size + 1, sizeof(char_t));
    GetEnvironmentVariable(name, buf, size + 1);
    return buf;
}

#ifndef UNICODE
#define CommandLineToArgv CommandLineToArgvA
extern char **CommandLineToArgvA(char *cmd_line, int *argc);
#else
#define CommandLineToArgv CommandLineToArgvA
#endif

#endif