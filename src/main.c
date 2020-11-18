#include "crt.h"
#include "logging.h"
#include "mono.h"

void doorstop_bootstrap(void *mono_domain) {
    mono.thread_set_main(mono.thread_current());

    if (mono.domain_set_config) {
        // TODO: set config
    }

    char *assembly_dir = mono.assembly_getrootdir();
    LOG("Assembly dir: %s\n", assembly_dir);

    // TODO: set env

    char *dll_path = NULL;
    char *app_path = NULL;

    LOG("Loading assembly: %s\n", dll_path);
    void *assembly = mono.domain_assembly_open(mono_domain, dll_path);

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
        void *str = mono.object_to_string(exc, NULL);
        char *exc_str = mono.string_to_utf8(str);
        LOG("Error message: %s\n", exc_str);
    }
    LOG("Done\n");

    mono.method_desc_free(desc);

    if (args != NULL) {
        free(app_path);
        free(args);
    }
}
