#include "../crt.h"

#define DEFINE_CALLS

// Remove warnings on some compilers about unneeded calling convention
#ifdef ENV64
#undef IMPORT_CONV
#define IMPORT_CONV
#endif

#define CAT2(start, middle, end) start##middle##end
#define CAT(start, middle, end) CAT2(start, middle, end)
#define STR2(A) #A
#define STR(A) STR2(A)
#define H(A) A.h
#define S2(A) A##_struct
#define S(A) S2(A)

#define IMPORT_LIB STR(H(IMPORT_PREFIX))
#define LOADER_FUNC_NAME CAT(load_, IMPORT_PREFIX, _funcs)

#define DEF_CALL(retType, name, ...)                                           \
    typedef retType(IMPORT_CONV *name##_t)(__VA_ARGS__);
#include IMPORT_LIB
#undef DEF_CALL

typedef struct {
#define DEF_CALL(retType, name, ...) name##_t name;
#include IMPORT_LIB
#undef DEF_CALL
} S(IMPORT_PREFIX);

extern S(IMPORT_PREFIX) IMPORT_PREFIX;

static void LOADER_FUNC_NAME(void *lib) {
#define DEF_CALL(retType, name, ...)                                           \
    IMPORT_PREFIX.name = (name##_t)dlsym(lib, STR(CAT(IMPORT_PREFIX, _, name)));
#include IMPORT_LIB
#undef DEF_CALL
}

#undef DEFINE_CALLS