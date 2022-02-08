#ifndef LOGGER_NIX_H
#define LOGGER_NIX_H
#if VERBOSE

#define LOG(message, ...) printf("[Doorstop] " message "\n", ##__VA_ARGS__)

#define ASSERT_F(test, message, ...)                                           \
    if (!(test)) {                                                             \
        printf("[Doorstop][Fatal] " message "\n", ##__VA_ARGS__);              \
        exit(1);                                                               \
    }

#define ASSERT(test, message)                                                  \
    if (!(test)) {                                                             \
        printf("[Doorstop][Fatal] " message "\n");                             \
        exit(1);                                                               \
    }

#define ASSERT_SOFT(test, ...)                                                 \
    if (!(test)) {                                                             \
        return __VA_ARGS__;                                                    \
    }

static inline void init_logger() {}
static inline void free_logger() {}

#endif
#endif