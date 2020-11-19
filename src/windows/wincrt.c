#include "wincrt.h"

#pragma function(memset)
void *memset(void *dst, int c, size_t n) {
    char *d = dst;
    while (n--)
        *d++ = (char)c;
    return dst;
}

#pragma function(memcpy)
void *memcpy(void *dst, const void *src, size_t n) {
    char *d = dst;
    const char *s = src;
    while (n--)
        *d++ = *s++;
    return dst;
}

#pragma function(wcslen)
size_t wcslen(wchar_t const *str) {
    size_t result = 0;
    while (*str++)
        result++;
    return result;
}

#pragma function(strlen)
size_t strlen(char const *str) {
    size_t result = 0;
    while (*str++)
        result++;
    return result;
}

void *malloc(size_t size) {
    return HeapAlloc(h_heap, HEAP_GENERATE_EXCEPTIONS, size);
}

void *calloc(size_t num, size_t size) {
    return HeapAlloc(h_heap, HEAP_ZERO_MEMORY, size * num);
}

inline wchar_t *wmemcpy(wchar_t *dst, const wchar_t *src, size_t n) {
    wchar_t *d = dst;
    const wchar_t *s = src;
    while (n--)
        *d++ = *s++;
    return dst;
}

inline void *wmemset(wchar_t *dst, wchar_t c, size_t n) {
    wchar_t *d = dst;
    while (n--)
        *d++ = c;
    return dst;
}

#ifndef UNICODE
LPSTR *CommandLineToArgvA(LPCSTR cmd_line, int *argc) {
    ULONG len = strlen(cmd_line);
    ULONG i = ((len + 2) / 2) * sizeof(LPVOID) + sizeof(LPVOID);

    LPSTR *argv =
        (LPSTR *)GlobalAlloc(GMEM_FIXED, i + (len + 2) * sizeof(CHAR));

    LPSTR _argv = (LPSTR)(((PUCHAR)argv) + i);

    ULONG _argc = 0;
    argv[_argc] = _argv;
    BOOL in_qm = FALSE;
    BOOL in_text = FALSE;
    BOOL in_space = TRUE;
    ULONG j = 0;
    i = 0;

    CHAR a;
    while (a = cmd_line[i]) {
        if (in_qm) {
            if (a == '\"') {
                in_qm = FALSE;
            } else {
                _argv[j] = a;
                j++;
            }
        } else {
            switch (a) {
            case '\"':
                in_qm = TRUE;
                in_text = TRUE;
                if (in_space) {
                    argv[_argc] = _argv + j;
                    _argc++;
                }
                in_space = FALSE;
                break;
            case ' ':
            case '\t':
            case '\n':
            case '\r':
                if (in_text) {
                    _argv[j] = '\0';
                    j++;
                }
                in_text = FALSE;
                in_space = TRUE;
                break;
            default:
                in_text = TRUE;
                if (in_space) {
                    argv[_argc] = _argv + j;
                    _argc++;
                }
                _argv[j] = a;
                j++;
                in_space = FALSE;
                break;
            }
        }
        i++;
    }
    _argv[j] = '\0';
    argv[_argc] = NULL;

    (*argc) = _argc;
    return argv;
}
#endif