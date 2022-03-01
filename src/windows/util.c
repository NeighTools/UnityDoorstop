#include "../util/util.h"
#include "wincrt.h"
#include <windows.h>

char *narrow(const char_t *str) {
#ifndef UNICODE
    char *result = (char *)malloc(strlen(str) + 1);
    strcpy(result, str);
    return result;
#else
    const int req_size =
        WideCharToMultiByte(CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL);
    char *result = malloc(req_size * sizeof(char));
    WideCharToMultiByte(CP_UTF8, 0, str, -1, result, req_size, NULL, NULL);
    return result;
#endif
}

char_t *widen(const char *str) {
#ifndef UNICODE
    char_t *result = (char_t *)malloc(strlen(str) + 1);
    strcpy(result, str);
    return result;
#else
    const int req_size = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
    wchar_t *result = malloc(req_size * sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, str, -1, result, req_size);
    return result;
#endif
}

size_t get_module_path(void *module, char_t **result, size_t *size,
                       size_t free_space) {
    DWORD i = 0;
    DWORD len, s;
    *result = NULL;
    do {
        if (*result != NULL)
            free(*result);
        i++;
        s = i * MAX_PATH + 1;
        *result = malloc(sizeof(char_t) * s);
        len = GetModuleFileName(module, *result, s);
    } while (GetLastError() == ERROR_INSUFFICIENT_BUFFER ||
             s - len < free_space);

    if (size != NULL)
        *size = s;
    return len;
}

char_t *get_full_path(char_t *path) {
    const DWORD needed = GetFullPathName(path, 0, NULL, NULL);
    char_t *res = malloc(sizeof(char_t) * needed);
    GetFullPathName(path, needed, res, NULL);
    return res;
}

bool_t file_exists(char_t *file) {
    DWORD ab = GetFileAttributes(file);
    return ab != INVALID_FILE_ATTRIBUTES &&
           (ab & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

bool_t folder_exists(char_t *folder) {
    DWORD ab = GetFileAttributes(folder);
    return ab != INVALID_FILE_ATTRIBUTES &&
           (ab & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

char_t *program_path() {
    char_t *app_path = NULL;
    get_module_path(NULL, &app_path, NULL, 0);
    return app_path;
}

typedef struct {
    size_t ext;
    size_t parent;
    size_t len;
} PathParts;

PathParts split_path(char_t *path) {
    size_t len = strlen(path);
    size_t ext = len;
    size_t i;
    for (i = len - 1; i > 0; i--) {
        char_t c = path[i];
        if (c == TEXT('.') && ext == len)
            ext = i;
        else if (c == TEXT('\\') || c == TEXT('/'))
            break;
    }
    return (PathParts){.ext = ext, .parent = i, .len = len};
}

char_t *get_folder_name(char_t *path) {
    PathParts parts = split_path(path);
    char_t *result = malloc((parts.parent + 1) * sizeof(char_t));
    strncpy(result, path, parts.parent);
    result[parts.parent] = TEXT('\0');
    return result;
}

char_t *get_file_name(char_t *path, bool_t with_ext) {
    PathParts parts = split_path(path);
    size_t result_len = (with_ext ? parts.len : parts.ext) - parts.parent;
    char_t *result = malloc(result_len * sizeof(char_t));
    strncpy(result, path + parts.parent + 1, result_len - 1);
    result[result_len - 1] = TEXT('\0');
    return result;
}

char_t *get_working_dir() {
    DWORD len = GetCurrentDirectory(0, NULL);
    char_t *result = malloc(sizeof(char_t) * len);
    GetCurrentDirectory(len, result);
    return result;
}

size_t get_file_size(void *file) { return (size_t)GetFileSize(file, NULL); }
