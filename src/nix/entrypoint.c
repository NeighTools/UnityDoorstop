#include "../bootstrap.h"
#include "../config/config.h"
#include "../crt.h"
#include "../util/logging.h"
#include "../util/paths.h"
#include "../util/util.h"
#include "./plthook/plthook.h"

extern char *program_invocation_name;
static void *(*real_dlsym)(void *, const char *) = NULL;
static int (*real_fclose)(FILE*) = NULL;

// Some crazy hackery here to get LD_PRELOAD hackery work with dlsym hooking
// Taken from https://stackoverflow.com/questions/15599026/how-can-i-intercept-dlsym-calls-using-ld-preload
extern void *_dl_sym(void *, const char *, void *);

#define real_dlsym real_dlsym
#define real_fclose real_fclose
#define dlsym_proxy dlsym
#define fclose_proxy fclose
#define program_path(app_path) realpath(program_invocation_name, app_path)
#define DYLD_INTERPOSE(_replacment, _replacee)
#define INIT_DLSYM                                                 \
{                                                                  \
        if (real_dlsym == NULL)                                    \
            real_dlsym = _dl_sym(RTLD_NEXT, "dlsym", dlsym_proxy); \
        if (!strcmp(name, "dlsym"))                                \
            return (void *)dlsym_proxy;                            \
}
#define INIT_FCLOSE                                                        \
{                                                                          \
        if (real_fclose == NULL)                                           \
            real_fclose = _dl_sym(RTLD_NEXT, "fclose", fclose_proxy);                 \
}

void capture_mono_path(void *handle) {
    char_t *result;
    get_module_path(handle, &result, NULL, 0);
    setenv("DOORSTOP_RUNTIME_LIB_PATH", result, TRUE);
}

static bool_t initialized = FALSE;
void *dlsym_proxy(void *handle, const char *name) {
    INIT_DLSYM;


#define REDIRECT_INIT(init_name, init_func, target, extra_init)                \
    if (!strcmp(name, init_name)) {                                            \
        if (!initialized) {                                                    \
            initialized = TRUE;                                                \
            init_func(handle);                                                 \
            extra_init;                                                        \
        }                                                                      \
        return (void *)target;                                                 \
    }

    REDIRECT_INIT("il2cpp_init", load_il2cpp_funcs, init_il2cpp, {});
    REDIRECT_INIT("mono_jit_init_version", load_mono_funcs, init_mono,
                  capture_mono_path(handle));
    REDIRECT_INIT("mono_image_open_from_data_with_name", load_mono_funcs,
                  hook_mono_image_open_from_data_with_name,
                  capture_mono_path(handle));
    REDIRECT_INIT("mono_jit_parse_options", load_mono_funcs,
                  hook_mono_jit_parse_options, capture_mono_path(handle));

#undef REDIRECT_INIT
    return real_dlsym(handle, name);
}

int fclose_proxy(FILE *stream) 
{
	INIT_FCLOSE;

	// Some versions of Unity wrongly close stdout, which prevents writing to console
	if (stream == stdout) 
		return F_OK;
	return real_fclose(stream);
}

__attribute__((constructor)) void doorstop_ctor() {
    init_logger();
    load_config();
}