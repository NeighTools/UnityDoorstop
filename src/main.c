#include "config.h"
#include "crt.h"
#include "logging.h"
#include "mono.h"
#include "util.h"

void doorstop_bootstrap(void *mono_domain) {
    if (!getenv(TEXT("DOORSTOP_INITIALIZED"))) {
        LOG("DOORSTOP_INITIALIZED is set! Skipping!");
        cleanup_config();
        return;
    }
    setenv(TEXT("DOORSTOP_INITIALIZED"), TEXT("TRUE"), TRUE);

    mono.thread_set_main(mono.thread_current());

    if (mono.domain_set_config) {
#define CONFIG_EXT TEXT(".config")
        char_t *exe_path = NULL;
        const size_t real_len =
            get_module_path(NULL, &exe_path, NULL, STR_LEN(CONFIG_EXT));
        char_t *folder_name = get_folder_name(exe_path, real_len, TRUE);
        strccpy(exe_path + real_len, CONFIG_EXT, STR_LEN(CONFIG_EXT));

        char *exe_path_n = narrow(exe_path);
        char *folder_path_n = narrow(folder_name);

        LOG("Setting config paths: base dir: %s; config path: %s\n",
            folder_path_n, exe_path_n);

        mono.domain_set_config(mono_domain, folder_path_n, exe_path_n);

        free(exe_path);
        free(folder_name);
        free(exe_path_n);
        free(folder_path_n);
#undef CONFIG_EXT
    }

    char *assembly_dir = mono.assembly_getrootdir();
    LOG("Assembly dir: %s\n", assembly_dir);

    char_t *norm_assembly_dir = widen(assembly_dir);
    setenv(TEXT("DOORSTOP_MANAGED_FOLDER_DIR"), norm_assembly_dir, TRUE);
    free(norm_assembly_dir);

    char *dll_path = narrow(config.target_assembly);

    char_t *app_path = NULL;
    get_module_path(NULL, &app_path, NULL, 0);
    setenv(TEXT("DOORSTOP_PROCESS_PATH"), app_path, TRUE);

    LOG("Loading assembly: %s\n", dll_path);
    void *assembly = mono.domain_assembly_open(mono_domain, dll_path);

    if (assembly == NULL)
        LOG("Failed to load assembly\n");

    free(dll_path);
    ASSERT_SOFT(assembly != NULL);

    void *image = mono.assembly_get_image(assembly);
    ASSERT_SOFT(image != NULL);

    void *desc = mono.method_desc_new("*:Main", FALSE);
    void *method = mono.method_desc_search_in_image(desc, image);
    ASSERT_SOFT(method != NULL);

    void *signature = mono.method_signature(method);

    unsigned int params = mono.signature_get_param_count(signature);

    void **args = NULL;
    if (params == 1) {
        void *args_array =
            mono.array_new(mono_domain, mono.get_string_class(), 0);
        args = malloc(sizeof(void *) * 1);
        args[0] = args_array;
    }

    LOG("Invoking method %p\n", method);
    void *exc = NULL;
    mono.runtime_invoke(method, NULL, args, &exc);
    if (exc != NULL) {
        LOG("Error invoking code!\n");
        if (mono.object_to_string) {
            void *str = mono.object_to_string(exc, NULL);
            char *exc_str = mono.string_to_utf8(str);
            LOG("Error message: %s\n", exc_str);
        }
    }
    LOG("Done\n");

    mono.method_desc_free(desc);

    if (args != NULL) {
        free(app_path);
        free(args);
    }

    cleanup_config();
}

void *init_mono(const char *root_domain_name, const char *runtime_version) {
    LOG("Starting mono domain \"%s\"", root_domain_name);
    void *domain = mono.jit_init_version(root_domain_name, runtime_version);
    doorstop_bootstrap(domain);
    return domain;
}