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

size_t strlen_wide(char_t const *str) {
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

char_t *strcat_wide(char_t *dst, const char_t *src) {
    size_t size = strlen(dst);
    size_t size2 = strlen(src);
    return strncpy(dst + size, src, size2 + 1);
}

char_t *strcpy_wide(char_t *dst, char_t *src) {
    char_t *d = dst;
    const char_t *s = src;
    while (*s)
        *d++ = *s++;
    *d = *s;
    return dst;
}

char_t *strncpy_wide(char_t *dst, const char_t *src, size_t n) {
    char_t *d = dst;
    const char_t *s = src;
    while (n--)
        *d++ = *s++;
    return dst;
}

char_t *dirname(char_t *path) {
    size_t len = strlen(path);
    size_t i;
    for (i = len; i > 0; i--) {
        char_t c = *(path + i);
        if (c == TEXT('\\') || c == TEXT('/'))
            break;
    }

    const size_t result_len = i;
    char_t *result = calloc(result_len + 1, sizeof(char_t));
    strncpy(result, path, result_len);
    return result;
}

char_t *getenv_wide(const char_t *name) {
    DWORD size = GetEnvironmentVariable(name, NULL, 0);
    if (size == 0)
        return NULL;
    char_t *buf = calloc(size + 1, sizeof(char_t));
    GetEnvironmentVariable(name, buf, size + 1);
    return buf;
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