#include "../util.h"
#include <windows.h>

char *narrow(const char_t *str) {
#ifndef UNICODE
    return str;
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
    return str;
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

char_t *get_folder_name(char_t *str, size_t len, int with_separator) {
    size_t i;
    for (i = len; i > 0; i--) {
        char_t c = *(str + i);
        if (c == TEXT('\\') || c == TEXT('/'))
            break;
    }

    const size_t result_len = i + (with_separator ? 1 : 0);
    char_t *result = calloc(result_len + 1, sizeof(char_t));
    strccpy(result, str, result_len);
    return result;
}

char_t *get_full_path(char_t *path) {
    const DWORD needed = GetFullPathName(path, 0, NULL, NULL);
    wchar_t *res = malloc(sizeof(char_t) * needed);
    GetFullPathName(path, needed, res, NULL);
    return res;
}

bool_t file_exists(char_t *file) {
    DWORD ab = GetFileAttributes(file);
    return ab != INVALID_FILE_ATTRIBUTES &&
           (ab & FILE_ATTRIBUTE_DIRECTORY) == 0;
}