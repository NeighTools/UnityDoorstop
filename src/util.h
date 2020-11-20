#ifndef UTIL_H
#define UTIL_H

#if _WIN32
#include <windows.h>
typedef TCHAR char_t;
typedef BOOL bool_t;

#elif defined(__APPLE__) || defined(__linux__)
typedef char char_t;
typedef int bool_t;

#define TEXT(text) text
#endif

#define STR_LEN(str) (sizeof(str) / sizeof((str)[0]))

char_t *widen(const char *str);
char *narrow(const char_t *str);
size_t get_module_path(void *module, char_t **result, size_t *size,
                       size_t free_space);
char_t *get_full_path(char_t *path);
bool_t file_exists(char_t *file);
void program_path(char_t **app_path);

#endif