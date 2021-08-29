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

size_t strlen_wide(const char_t *str);
#define strlen strlen_wide

void *malloc(size_t size);

void *calloc(size_t num, size_t size);

char_t *strcat_wide(char_t *dst, const char_t *src);
#define strcat strcat_wide

char_t *strcpy_wide(char_t *dst, const char_t *src);
#define strcpy strcpy_wide

char_t *strncpy_wide(char_t *dst, const char_t *src, size_t len);
#define strncpy strncpy_wide

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

char_t *getenv_wide(const char_t *name);
#define getenv getenv_wide

void shutenv(char_t *val);

void *fopen(char_t *filename, const char_t *mode);
size_t fread(void *ptr, size_t size, size_t count, void *stream);
int fclose(void *stream);

#ifndef UNICODE
#define CommandLineToArgv CommandLineToArgvA
LPSTR *CommandLineToArgvA(LPCSTR cmd_line, int *argc);

#define strcmpi lstrcmpiA
#else
#define CommandLineToArgv CommandLineToArgvW
#define strcmpi lstrcmpiW
#endif

#endif