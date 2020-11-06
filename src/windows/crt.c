#include "crt.h"

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
