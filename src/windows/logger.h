#ifndef LOGGER_H
#define LOGGER_H
#if VERBOSE

#include "../logging.h"
#include "../util.h"
#include <windows.h>

HANDLE log_handle;
char_t buffer[4096];

#ifdef UNICODE
#define printf wsprintfW
#else
#define printf wsprintfA
#endif

inline void init_logger() {
    printf(buffer, TEXT("doorstop_%lx.log"), GetTickCount64());
    log_handle = CreateFile(buffer, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                            CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
}

inline void free_logger() { CloseHandle(log_handle); }

#define LOG(message, ...)                                                      \
    {                                                                          \
        size_t len = printf(buffer, TEXT(message), __VA_ARGS__);               \
        WriteFile(log_handle, buffer, len, NULL, NULL);                        \
    }

#define ASSERT_F(test, message, ...)                                           \
    if (!(test)) {                                                             \
        char_t *buff = (char_t *)malloc(sizeof(char_t) * 1024);                \
        printf(buff, TEXT(message), __VA_ARGS__);                              \
        MessageBox(NULL, buff, TEXT("Doorstop: Fatal"), MB_OK | MB_ICONERROR); \
        free(buff);                                                            \
        ExitProcess(EXIT_FAILURE);                                             \
    }

#define ASSERT(test, message)                                                  \
    if (!(test)) {                                                             \
        MessageBox(NULL, TEXT(message), TEXT("Doorstop: Fatal"),               \
                   MB_OK | MB_ICONERROR);                                      \
        ExitProcess(EXIT_FAILURE);                                             \
    }

#define ASSERT_SOFT(test, ...)                                                 \
    if (!(test)) {                                                             \
        return __VA_ARGS__;                                                    \
    }

#endif
#endif