#include "../util/util.h"
#include "../crt.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char_t *widen(const char *str) {
    char_t *result = (char_t *)malloc(strlen(str) + 1);
    strcpy(result, str);
    return result;
}

char *narrow(const char_t *str) {
    char *result = (char *)malloc(strlen(str) + 1);
    strcpy(result, str);
    return result;
}

size_t get_module_path(void *module, char_t **result, size_t *size,
                       size_t free_space) {
    Dl_info info;
    dladdr(module, &info);
    size_t len = strlen(info.dli_fname);
    size_t total_size = len + free_space + 1;
    if (size != NULL)
        *size = len;
    *result = (char_t *)malloc(total_size);
    strcpy(*result, info.dli_fname);
    return total_size;
}

#if defined(__APPLE__)
void *memrchr(const void *s, int c, size_t n) {
    const char *p = (const char *)s + n;
    while (--p >= (const char *)s) {
        if (*p == c)
            return (void *)p;
    }
    return NULL;
}
#endif

// Adapted from: https://gist.github.com/Eugeny/5127791
/**
 * @brief Normalize the path in respect to the current working directory.
 *
 * @param pwd Current working directory.
 * @param src Path to normalize.
 * @param res Buffer to store the normalized path.
 * @return char* Pointer to the normalized path (same as res).
 */
char *normalize_path(char *pwd, const char *src, char *res) {
    size_t res_len;
    size_t src_len = strlen(src);

    const char *ptr = src;
    const char *end = &src[src_len];
    const char *next;

    if (src_len == 0 || src[0] != '/') {
        // relative path
        size_t pwd_len;

        pwd_len = strlen(pwd);
        memcpy(res, pwd, pwd_len);
        res_len = pwd_len;
    } else {
        res_len = 0;
    }

    for (ptr = src; ptr < end; ptr = next + 1) {
        size_t len;
        next = (char *)memchr(ptr, '/', end - ptr);
        if (next == NULL) {
            next = end;
        }
        len = next - ptr;
        switch (len) {
        case 2:
            if (ptr[0] == '.' && ptr[1] == '.') {
                const char *slash = (char *)memrchr(res, '/', res_len);
                if (slash != NULL) {
                    res_len = slash - res;
                }
                continue;
            }
            break;
        case 1:
            if (ptr[0] == '.') {
                continue;
            }
            break;
        case 0:
            continue;
        }

        if (res_len != 1)
            res[res_len++] = '/';

        memcpy(&res[res_len], ptr, len);
        res_len += len;
    }

    if (res_len == 0) {
        res[res_len++] = '/';
    }
    res[res_len] = '\0';
    return res;
}

char_t *get_full_path(char_t *path) {
    char_t *full_path = (char_t *)calloc(MAX_PATH + 1, sizeof(char_t));
    char_t *cwd_str = getcwd(NULL, MAX_PATH);
    normalize_path(cwd_str, path, full_path);
    free(cwd_str);
    if (full_path == NULL) {
        free(full_path);
        return NULL;
    }
    return full_path;
}

char_t *get_folder_name(char_t *path) {
    char_t *path_copy = (char_t *)calloc(MAX_PATH + 1, sizeof(char_t));
    strcpy(path_copy, path);
    char_t *folder = dirname(path_copy);
    char_t *result = (char_t *)malloc(strlen(folder) + 1);
    strcpy(result, folder);
    free(path_copy);
    return result;
}

char_t *get_file_name(char_t *path, bool_t with_ext) {
    char_t *path_copy = (char_t *)calloc(MAX_PATH + 1, sizeof(char_t));
    strcpy(path_copy, path);
    char_t *file = basename(path_copy);
    char_t *result = (char_t *)calloc(strlen(file) + 1, sizeof(char_t));
    if (!with_ext) {
        strcpy(result, file);
        char_t *ext = strrchr(file, '.');
        if (ext != NULL) {
            *ext = '\0';
        }
    }
    strcpy(result, file);
    free(path_copy);
    return result;
}

bool_t file_exists(char_t *file) { return access(file, F_OK) == 0; }

bool_t folder_exists(char_t *folder) {
    struct stat sb;
    return stat(folder, &sb) == 0 && S_ISDIR(sb.st_mode);
}

char_t *get_working_dir() { return getcwd(NULL, 0); }

char_t *program_path() {
#if defined(__linux__)
    char_t *path = (char_t *)calloc(MAX_PATH + 1, sizeof(char_t));
    if (readlink("/proc/self/exe", path, MAX_PATH) == -1) {
        free(path);
        return NULL;
    }
    return path;
#elif defined(__APPLE__)
    char_t *path = (char_t *)calloc(MAX_PATH + 1, sizeof(char_t));
    uint32_t size = MAX_PATH;
    if (_NSGetExecutablePath(path, &size) != 0) {
        free(path);
        return NULL;
    }
    return path;
#endif
}

size_t get_file_size(void *file) {
    struct stat sb;
    if (fstat(fileno(file), &sb) == -1) {
        return 0;
    }
    return sb.st_size;
}