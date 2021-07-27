#include "config.h"
#include "crt.h"
#include "il2cpp.h"
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
        char_t *app_path = NULL;
        program_path(&app_path);
        char_t *config_path =
            calloc(strlen(app_path) + 1 + STR_LEN(CONFIG_EXT), sizeof(char_t));
        strcpy(config_path, app_path);

        strcat(config_path, CONFIG_EXT);
        char_t *folder_path = get_folder_name(app_path);

        char *config_path_n = narrow(config_path);
        char *folder_path_n = narrow(folder_path);

        LOG("Setting config paths: base dir: %s; config path: %s\n",
            folder_path_n, config_path_n);

        mono.domain_set_config(mono_domain, folder_path_n, config_path_n);

        free(app_path);
        free(folder_path);
        free(config_path);
        free(config_path_n);
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

int init_il2cpp(const char *domain_name) {
    LOG("Starting IL2CPP domain \"%s\"\n", domain_name);
    const int orig_result = il2cpp.init(domain_name);

    char_t *mono_lib_dir = get_full_path(config.mono_lib_dir);
    char_t *mono_corlib_dir = get_full_path(config.mono_corlib_dir);
    char_t *mono_config_dir = get_full_path(config.mono_config_dir);

    LOG("Mono lib: %S\n", mono_lib_dir);
    LOG("Mono mscorlib dir: %S\n", mono_corlib_dir);
    LOG("Mono confgi dir: %S\n", mono_config_dir);

    if (!file_exists(mono_lib_dir) || !folder_exists(mono_corlib_dir) ||
        !folder_exists(mono_config_dir)) {
        LOG("Mono startup dirs are not set up, skipping invoking Doorstop\n");
        return orig_result;
    }

    void *mono_module = dlopen(mono_lib_dir, RTLD_LAZY);
    LOG("Loaded mono.dll: %p\n", mono_module);
    if (!mono_module) {
        LOG("Failed to load mono.dll! Skipping!");
        return orig_result;
    }

    load_mono_funcs(mono_module);
    LOG("Loaded mono.dll functions\n");

    char *mono_corlib_dir_narrow = narrow(mono_corlib_dir);
    char *mono_config_dir_narrow = narrow(mono_config_dir);
    mono.set_dirs(mono_corlib_dir_narrow, mono_config_dir_narrow);
    mono.config_parse(NULL);

    void *domain = mono.jit_init_version("Doorstop Root Domain", NULL);
    LOG("Created domain: %p\n", domain);

    doorstop_bootstrap(domain);

    return orig_result;
}