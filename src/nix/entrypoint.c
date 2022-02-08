#include "../bootstrap.h"
#include "../config/config.h"
#include "../crt.h"
#include "../util/paths.h"
#include "../util/util.h"
#include "./plthook/plthook.h"

static bool_t initialized = FALSE;

void *dlsym_hook(void *handle, const char *name) {
#define REDIRECT_INIT(init_name, init_func, target)                            \
    if (!strcmp(name, init_name)) {                                            \
        if (!initialized) {                                                    \
            init_func(handle);                                                 \
            initialized = TRUE;                                                \
        }                                                                      \
        return (void *)target;                                                 \
    }

    REDIRECT_INIT("il2cpp_init", load_il2cpp_funcs, init_il2cpp);
    REDIRECT_INIT("mono_jit_init_version", load_mono_funcs, init_mono);
    REDIRECT_INIT("mono_image_open_from_data_with_name", load_mono_funcs,
                  hook_mono_image_open_from_data_with_name);
#undef REDIRECT_INIT
    return dlsym(handle, name);
}

int fclose_hook(FILE *stream) {
    // Some versions of Unity wrongly close stdout, which prevents writing
    // to console
    if (stream == stdout)
        return F_OK;
    return fclose(stream);
}

__attribute__((constructor)) void doorstop_ctor() {
    init_logger();
    if (strcmp(getenv("DOORSTOP_ENABLE"), "TEXT")) {
        LOG("DOORSTOP_ENABLE is set! Skipping!\n");
        return;
    }

    load_config();

    plthook_t *hook;

    void *unity_player = plthook_handle_by_name("UnityPlayer");

#if defined(__linux__)
    if (!unity_player) {
        unity_player = dlopen("UnityPlayer.so", RTLD_LAZY);
    }
#endif

    if (unity_player && plthook_open_by_handle(&hook, unity_player) == 0) {
        LOG("Found UnityPlayer, hooking into it instead\n");
    } else if (plthook_open(&hook, NULL) != 0) {
        LOG("Failed to open current process PLT! Cannot run Doorstop! "
            "Error: "
            "%s\n",
            plthook_error());
        return;
    }

    if (plthook_replace(hook, "dlsym", &dlsym_hook, NULL) != 0)
        printf("Failed to hook dlsym, ignoring it. Error: %s\n",
               plthook_error());

    if (plthook_replace(hook, "fclose", &fclose_hook, NULL) != 0)
        printf("Failed to hook fclose, ignoring it. Error: %s\n",
               plthook_error());

#if defined(__APPLE__)
    /*
        On older Unity versions, Mono methods are resolved by the OS's
       loader directly. Because of this, there is no dlsym, in which case we
       need to apply a PLT hook.
    */
    void *mono_handle = plthook_handle_by_name("libmono");

    if (plthook_replace(hook, "mono_jit_init_version", &jit_init_hook, NULL) !=
        0)
        printf("Failed to hook jit_init_version, ignoring it. Error: %s\n",
               plthook_error());
    else if (mono_handle)
        doorstop_init_mono_functions(mono_handle);
#endif

    plthook_close(hook);
}