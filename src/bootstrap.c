#include "bootstrap.h"
#include "config/config.h"
#include "crt.h"
#include "runtimes/coreclr.h"
#include "runtimes/il2cpp.h"
#include "runtimes/mono.h"
#include "util/logging.h"
#include "util/paths.h"
#include "util/util.h"

void mono_doorstop_bootstrap(void *mono_domain) {
    if (getenv(TEXT("DOORSTOP_INITIALIZED"))) {
        LOG("DOORSTOP_INITIALIZED is set! Skipping!\n");
        cleanup_config();
        return;
    }
    setenv(TEXT("DOORSTOP_INITIALIZED"), TEXT("TRUE"), TRUE);

    mono.thread_set_main(mono.thread_current());

    char_t *app_path = program_path();
    if (mono.domain_set_config) {
#define CONFIG_EXT TEXT(".config")
        char_t *config_path =
            calloc(strlen(app_path) + 1 + STR_LEN(CONFIG_EXT), sizeof(char_t));
        strcpy(config_path, app_path);
        strcat(config_path, CONFIG_EXT);
        char_t *folder_path = get_folder_name(app_path);

        char *config_path_n = narrow(config_path);
        char *folder_path_n = narrow(folder_path);

        LOG("Setting config paths: base dir: %s; config path: %s\n",
            folder_path, config_path);

        mono.domain_set_config(mono_domain, folder_path_n, config_path_n);

        free(folder_path);
        free(config_path);
        free(config_path_n);
        free(folder_path_n);
#undef CONFIG_EXT
    }

    setenv(TEXT("DOORSTOP_INVOKE_DLL_PATH"), config.target_assembly, TRUE);
    setenv(TEXT("DOORSTOP_PROCESS_PATH"), app_path, TRUE);

    char *assembly_dir = mono.assembly_getrootdir();
    char_t *norm_assembly_dir = widen(assembly_dir);

    mono.config_parse(NULL);

    LOG("Assembly dir: %s\n", norm_assembly_dir);
    setenv(TEXT("DOORSTOP_MANAGED_FOLDER_DIR"), norm_assembly_dir, TRUE);
    free(norm_assembly_dir);

    LOG("Opening assembly: %s\n", config.target_assembly);
    void *file = fopen(config.target_assembly, "r");
    if (!file) {
        LOG("Failed to open assembly: %s\n", config.target_assembly);
        return;
    }

    size_t size = get_file_size(file);
    void *data = malloc(size);
    fread(data, size, 1, file);
    fclose(file);

    LOG("Opened Assembly DLL (%d bytes); opening its main image\n", size);

    char *dll_path = narrow(config.target_assembly);
    MonoImageOpenStatus s = MONO_IMAGE_OK;
    void *image = mono.image_open_from_data_with_name(data, size, TRUE, &s,
                                                      FALSE, dll_path);
    free(data);
    if (s != MONO_IMAGE_OK) {
        LOG("Failed to load assembly image: %s. Got result: %d\n",
            config.target_assembly, s);
        return;
    }
    free(dll_path);

    LOG("Image opened; loading included assembly\n");

    s = MONO_IMAGE_OK;
    void *assembly = mono.assembly_load_from_full(image, dll_path, &s, FALSE);

    if (s != MONO_IMAGE_OK) {
        LOG("Failed to load assembly: %s. Got result: %d\n",
            config.target_assembly, s);
        return;
    }

    LOG("Assembly loaded; looking for Doorstop.Entrypoint:Start\n");
    void *desc = mono.method_desc_new("Doorstop.Entrypoint:Start", TRUE);
    void *method = mono.method_desc_search_in_image(desc, image);
    mono.method_desc_free(desc);
    if (!method) {
        LOG("Failed to find method Doorstop.Entrypoint:Start\n");
        return;
    }

    void *signature = mono.method_signature(method);
    unsigned int params = mono.signature_get_param_count(signature);
    if (params != 0) {
        LOG("Method has %d parameters; expected 0\n", params);
        return;
    }

    LOG("Invoking method %p\n", method);
    void *exc = NULL;
    mono.runtime_invoke(method, NULL, NULL, &exc);
    if (exc != NULL) {
        LOG("Error invoking code!\n");
        if (mono.object_to_string) {
            void *str = mono.object_to_string(exc, NULL);
            char *exc_str = mono.string_to_utf8(str);
            LOG("Error message: %s\n", exc_str);
        }
    }
    LOG("Done\n");

    free(app_path);
    cleanup_config();
}

void *init_mono(const char *root_domain_name, const char *runtime_version) {
    char_t *root_domain_name_w = widen(root_domain_name);
    LOG("Starting mono domain \"%s\"\n", root_domain_name_w);
    free(root_domain_name_w);
    char *root_dir_n = mono.assembly_getrootdir();
    char_t *root_dir = widen(root_dir_n);
    LOG("Current root: %s\n", root_dir);

    LOG("Overriding mono DLL search path\n");

    size_t mono_search_path_len = strlen(root_dir) + 1;
    char_t *target_path_full = get_full_path(config.target_assembly);
    char_t *target_path_folder = get_folder_name(target_path_full);
    mono_search_path_len += strlen(target_path_folder) + 1;
    LOG("Adding %s to mono search path\n", target_path_folder);

    char_t *override_dir_full = NULL;
    bool_t has_override = config.mono_dll_search_path_override && strlen(config.mono_dll_search_path_override);
    if (has_override) {
        override_dir_full = get_full_path(config.mono_dll_search_path_override);
        mono_search_path_len += strlen(override_dir_full) + 1;
        LOG("Adding root path: %s\n", override_dir_full);
    }

    char_t *mono_search_path =
        calloc(mono_search_path_len + 1, sizeof(char_t));
    if (has_override) {
        strcat(mono_search_path, override_dir_full);
        strcat(mono_search_path, PATH_SEP);
    }
    strcat(mono_search_path, target_path_folder);
    strcat(mono_search_path, PATH_SEP);
    strcat(mono_search_path, root_dir);

    LOG("Mono search path: %s\n", mono_search_path);
    char *mono_search_path_n = narrow(mono_search_path);
    mono.set_assemblies_path(mono_search_path_n);
    setenv(TEXT("DOORSTOP_DLL_SEARCH_DIRS"), mono_search_path, TRUE);
    free(mono_search_path);
    free(mono_search_path_n);
    if (override_dir_full) {
        free(override_dir_full);
    }
    free(target_path_full);
    free(target_path_folder);

    hook_mono_jit_parse_options(0, NULL);

    void *domain = mono.jit_init_version(root_domain_name, runtime_version);

    if (config.mono_debug_enabled && config.mono_debug_start_server) {
        mono.debug_init(MONO_DEBUG_FORMAT_MONO);
        mono.debug_domain_create(domain);
    }

    mono_doorstop_bootstrap(domain);

    return domain;
}

void il2cpp_doorstop_bootstrap() {
    if (!config.clr_corlib_dir || !config.clr_runtime_coreclr_path) {
        LOG("No CoreCLR paths set, skipping loading\n");
        return;
    }

    LOG("CoreCLR runtime path: %s\n", config.clr_runtime_coreclr_path);
    LOG("CoreCLR corlib dir: %s\n", config.clr_corlib_dir);

    if (!file_exists(config.clr_runtime_coreclr_path) ||
        !folder_exists(config.clr_corlib_dir)) {
        LOG("CoreCLR startup dirs are not set up skipping invoking Doorstop\n");
        return;
    }

    void *coreclr_module = dlopen(config.clr_runtime_coreclr_path, RTLD_LAZY);
    LOG("Loaded coreclr.dll: %p\n", coreclr_module);
    if (!coreclr_module) {
        LOG("Failed to load CoreCLR runtime!\n");
        return;
    }

    load_coreclr_funcs(coreclr_module);

    char_t *app_path;
    get_module_path(NULL, &app_path, NULL, 0);
    char *app_path_n = narrow(app_path);

    char_t *target_dir = get_folder_name(config.target_assembly);
    char *target_dir_n = narrow(target_dir);
    char_t *target_name = get_file_name(config.target_assembly, FALSE);
    char *target_name_n = narrow(target_name);

    char_t *app_paths_env =
        calloc(strlen(config.clr_corlib_dir) + 1 + strlen(target_dir) + 1,
               sizeof(char_t));
    strcat(app_paths_env, config.clr_corlib_dir);
    strcat(app_paths_env, PATH_SEP);
    strcat(app_paths_env, target_dir);
    char *app_paths_env_n = narrow(app_paths_env);

    LOG("App path: %s\n", app_path);
    LOG("Target dir: %s\n", target_dir);
    LOG("Target name: %s\n", target_name);
    LOG("APP_PATHS: %s\n", app_paths_env);

    char *props = "APP_PATHS";

    setenv(TEXT("DOORSTOP_INITIALIZED"), TEXT("TRUE"), TRUE);
    setenv(TEXT("DOORSTOP_INVOKE_DLL_PATH"), config.target_assembly, TRUE);
    setenv(TEXT("DOORSTOP_MANAGED_FOLDER_DIR"), config.clr_corlib_dir, TRUE);
    setenv(TEXT("DOORSTOP_PROCESS_PATH"), app_path, TRUE);

    void *host = NULL;
    unsigned int domain_id = 0;
    int result = coreclr.initialize(app_path_n, "Doorstop Domain", 1, &props,
                                    &app_paths_env_n, &host, &domain_id);
    if (result != 0) {
        LOG("Failed to initialize CoreCLR: %d\n", result);
        return;
    }

    void (*startup)() = NULL;
    result = coreclr.create_delegate(host, domain_id, target_name_n,
                                     "Doorstop.Entrypoint", "Start", &startup);
    if (result != 0) {
        LOG("Failed to get entrypoint delegate: %d\n", result);
        return;
    }

    LOG("Invoking Doorstop.Entrypoint.Start()\n");
    startup();
}

int init_il2cpp(const char *domain_name) {
    char_t *domain_name_w = widen(domain_name);
    LOG("Starting IL2CPP domain \"%s\"\n", domain_name_w);
    free(domain_name_w);
    const int orig_result = il2cpp.init(domain_name);
    il2cpp_doorstop_bootstrap();
    return orig_result;
}

#define MONO_DEBUG_ARG_START                                                   \
    TEXT("--debugger-agent=transport=dt_socket,server=y,address=")
// TODO: For .NET 3.5 monos, need to use defer=y instead
#define MONO_DEBUG_NO_SUSPEND TEXT(",suspend=n")

void hook_mono_jit_parse_options(int argc, char **argv) {
    char_t *debug_options = getenv(TEXT("DNSPY_UNITY_DBG2"));
    if (debug_options) {
        config.mono_debug_enabled = TRUE;
    }

    if (config.mono_debug_enabled) {
        LOG("Configuring mono debug server\n");

        int size = argc + 1;
        char **new_argv = calloc(size, sizeof(char *));
        memcpy(new_argv, argv, argc * sizeof(char *));

        size_t debug_args_len =
            STR_LEN(MONO_DEBUG_ARG_START) + strlen(config.mono_debug_address);
        if (!config.mono_debug_suspend) {
            debug_args_len += STR_LEN(MONO_DEBUG_NO_SUSPEND);
        }

        if (!debug_options) {
            debug_options = calloc(debug_args_len + 1, sizeof(char_t));
            strcat(debug_options, MONO_DEBUG_ARG_START);
            strcat(debug_options, config.mono_debug_address);
            if (!config.mono_debug_suspend) {
                strcat(debug_options, MONO_DEBUG_NO_SUSPEND);
            }
        }

        LOG("Debug options: %s\n", debug_options);

        char *debug_options_n = narrow(debug_options);
        new_argv[argc] = debug_options_n;
        mono.jit_parse_options(size, new_argv);

        free(debug_options);
        free(debug_options_n);
        free(new_argv);
    } else {
        mono.jit_parse_options(argc, argv);
    }
}

void *hook_mono_image_open_from_data_with_name(void *data,
                                               unsigned long data_len,
                                               int need_copy,
                                               MonoImageOpenStatus *status,
                                               int refonly, const char *name) {
    void *result = NULL;
    if (config.mono_dll_search_path_override) {
        char_t *name_wide = widen(name);
        char_t *name_file = get_file_name(name_wide, TRUE);
        free(name_wide);

        size_t name_file_len = strlen(name_file);
        size_t bcl_root_len = strlen(config.mono_dll_search_path_override);

        char_t *new_full_path =
            calloc(name_file_len + bcl_root_len + 2, sizeof(char_t));
        strcat(new_full_path, config.mono_dll_search_path_override);
        strcat(new_full_path, TEXT("/"));
        strcat(new_full_path, name_file);

        if (file_exists(new_full_path)) {
            void *file = fopen(new_full_path, "r");
            size_t size = get_file_size(file);
            void *buf = malloc(size);
            fread(buf, 1, size, file);
            fclose(file);
            result = mono.image_open_from_data_with_name(buf, size, need_copy,
                                                         status, refonly, name);
            if (need_copy)
                free(buf);
        }
        free(new_full_path);
    }

    if (!result) {
        result = mono.image_open_from_data_with_name(data, data_len, need_copy,
                                                     status, refonly, name);
    }
    return result;
}