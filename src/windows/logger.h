#ifndef LOGGER_WIN_H
#define LOGGER_WIN_H
#if VERBOSE

#include "../util/logging.h"
#include "../util/util.h"
#include <windows.h>

extern HANDLE log_handle;
extern char_t buffer[4096];

#ifdef UNICODE
#define printf wsprintfW
#else
#define printf wsprintfA
#endif

static inline void init_logger() {
    printf(buffer, TEXT("doorstop_%lx.log"), GetTickCount());
    log_handle = CreateFile(buffer, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                            CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
}

static inline void free_logger() { CloseHandle(log_handle); }

#if !defined(_MSVC_TRADITIONAL) || _MSVC_TRADITIONAL
#define LOG(message, ...)                                                      \
    {                                                                          \
        size_t len = printf(buffer, TEXT(message) TEXT("\n"), ##__VA_ARGS__);  \
        char *log_data = narrow(buffer);                                       \
        WriteFile(log_handle, log_data, len, NULL, NULL);                      \
        free(log_data);                                                        \
    }
#else
#define LOG(message, ...)                                                      \
    {                                                                          \
        size_t len = printf(buffer, TEXT(message) TEXT("\n") __VA_OPT__(, ) __VA_ARGS__); \
        char *log_data = narrow(buffer);                                       \
        WriteFile(log_handle, log_data, len, NULL, NULL);                      \
        free(log_data);                                                        \
    }
#endif

#define ASSERT_F(test, message, ...)                                           \
    if (!(test)) {                                                             \
        char_t *buff = (char_t *)malloc(sizeof(char_t) * 1024);                \
        printf(buff, TEXT(message) TEXT("\n"), __VA_ARGS__);                              \
        MessageBox(NULL, buff, TEXT("Doorstop: Fatal"), MB_OK | MB_ICONERROR); \
        free(buff);                                                            \
        ExitProcess(EXIT_FAILURE);                                             \
    }

#define ASSERT(test, message)                                                  \
    if (!(test)) {                                                             \
        MessageBox(NULL, TEXT(message) TEXT("\n"), TEXT("Doorstop: Fatal"),               \
                   MB_OK | MB_ICONERROR);                                      \
        ExitProcess(EXIT_FAILURE);                                             \
    }

#define ASSERT_SOFT(test, ...)                                                 \
    if (!(test)) {                                                             \
        return __VA_ARGS__;                                                    \
    }

#endif
#endif