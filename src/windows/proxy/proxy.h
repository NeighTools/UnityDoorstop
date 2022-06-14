#ifndef PROXY_H
#define PROXY_H

#include "../../crt.h"
#include "../../util/logging.h"
#include "../../util/util.h"
#include <windows.h>

#define ALT_POSTFIX TEXT("_alt.dll")
#define DLL_POSTFIX TEXT(".dll")

extern void load_functions(void *dll);

static inline void load_proxy(char_t *module_name) {
    size_t module_name_len = strlen(module_name);

    char_t *alt_name = (char_t *)malloc(
        (module_name_len + STR_LEN(ALT_POSTFIX)) * sizeof(char_t));
    strcat(strcpy(alt_name, module_name), ALT_POSTFIX);

    char_t *alt_full_path = get_full_path(alt_name);
    free(alt_name);

    LOG("Looking for original DLL from %s", alt_full_path);

    void *handle = dlopen(alt_full_path, RTLD_LAZY);
    free(alt_full_path);

    if (handle == NULL) {
        UINT sys_len = GetSystemDirectory(NULL, 0);
        char_t *sys_full_path = (char_t *)malloc(
            (sys_len + module_name_len + STR_LEN(DLL_POSTFIX)) *
            sizeof(char_t));
        GetSystemDirectory(sys_full_path, sys_len);
        sys_full_path[sys_len - 1] = TEXT('\\');
        strcpy(strcpy(sys_full_path + sys_len, module_name) + module_name_len,
               DLL_POSTFIX);

        LOG("Looking for original DLL from %s", sys_full_path);

        handle = dlopen(sys_full_path, RTLD_LAZY);
        free(sys_full_path);
    }

    ASSERT_F(handle != NULL, "Unable to load original %s.dll!", module_name);

    load_functions(handle);
}

#endif