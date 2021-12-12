/*
 * Custom implementation for common C runtime functions
 * This makes the DLL essentially freestanding on Windows without having to rely
 * on msvcrt.dll
 */
#ifndef WIN_CRT_H
#define WIN_CRT_H

#include "../util/util.h"
#include <windows.h>

// Fix for MinGW's headers
#if UNICODE
#define GetFinalPathNameByHandle GetFinalPathNameByHandleW
#else
#define GetFinalPathNameByHandle GetFinalPathNameByHandleA
#endif

extern void init_crt();

#define STR_LEN(str) (sizeof(str) / sizeof((str)[0]))

extern void *memset(void *dst, int c, size_t n);
#pragma intrinsic(memset)

extern void *memcpy(void *dst, const void *src, size_t n);
#pragma intrinsic(memcpy)

extern size_t strlen_wide(const char_t *str);
#define strlen strlen_wide

extern void *malloc(size_t size);

extern void *calloc(size_t num, size_t size);

extern char_t *strcat_wide(char_t *dst, const char_t *src);
#define strcat strcat_wide

extern char_t *strcpy_wide(char_t *dst, const char_t *src);
#define strcpy strcpy_wide

extern char_t *strncpy_wide(char_t *dst, const char_t *src, size_t len);
#define strncpy strncpy_wide

extern void *dlsym(void *handle, const char *name);

#define RTLD_LAZY 0x00001

extern void *dlopen(const char_t *filename, int flag);

extern void free(void *mem);

extern int setenv(const char_t *name, const char_t *value, int overwrite);
extern char_t *getenv_wide(const char_t *name);
#define getenv getenv_wide

extern void shutenv(char_t *val);

extern void *fopen(char_t *filename, const char_t *mode);
extern size_t fread(void *ptr, size_t size, size_t count, void *stream);
extern int fclose(void *stream);

#ifndef UNICODE
#define CommandLineToArgv CommandLineToArgvA
extern LPSTR *CommandLineToArgvA(LPCSTR cmd_line, int *argc);

#define strcmpi lstrcmpiA
#else
#define CommandLineToArgv CommandLineToArgvW
#define strcmpi lstrcmpiW
#endif

#endif