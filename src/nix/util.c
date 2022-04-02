#include "../util/util.h"
#include "../crt.h"

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
    size_t total_size = len + free_space;
    if (size != NULL)
        *size = len;
    *result = (char_t *)malloc(total_size);
    strcpy(*result, info.dli_fname);
    return total_size;
}

char_t *get_full_path(char_t *path) {
    char_t *buf = (char_t *)malloc(MAX_PATH);
    char_t *full_path = realpath(path, buf);
    if (full_path == NULL) {
        free(full_path);
        return NULL;
    }
    return full_path;
}

char_t *get_folder_name(char_t *path) {
    char_t *path_copy = (char_t *)malloc(MAX_PATH);
    strcpy(path_copy, path);
    char_t *folder = dirname(path_copy);
    char_t *result = (char_t *)malloc(strlen(folder) + 1);
    strcpy(result, folder);
    free(path_copy);
    return result;
}

char_t *get_file_name(char_t *path, bool_t with_ext) {
    char_t *path_copy = (char_t *)malloc(MAX_PATH);
    strcpy(path_copy, path);
    char_t *file = basename(path_copy);
    char_t *result = (char_t *)malloc(strlen(file) + 1);
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
    char_t *path = (char_t *)malloc(MAX_PATH);
    if (readlink("/proc/self/exe", path, MAX_PATH) == -1) {
        free(path);
        return NULL;
    }
    return path;
#elif defined(__APPLE__)
    char_t *path = (char_t *)malloc(MAX_PATH);
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