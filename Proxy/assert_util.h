#pragma once

#include "ver.h"
#include <windows.h>

#ifdef _VERBOSE
#include <stdio.h>

static FILE *log_handle;

inline void init_logger()
{
	fopen_s(&log_handle, "doorstop.log", "w");
	setvbuf(log_handle, NULL, _IONBF, 0);
}

inline void free_logger()
{
	fclose(log_handle);
}

#define LOG(message, ...) fwprintf_s(log_handle, message, __VA_ARGS__)
#define LOGA(message, ...) fprintf_s(log_handle, message, __VA_ARGS__)
#else
inline void init_logger() {}
inline void free_logger() {}
#define LOG(message, ...)
#define LOGA(message, ...)
#endif

#define ASSERT_F(test, message, ...)												\
	if(!(test))																		\
	{																				\
		wchar_t *buff = (wchar_t*)malloc(sizeof(wchar_t) * 1024);					\
		swprintf_s(buff, 1024, message, __VA_ARGS__);								\
		MessageBox(NULL, buff, L"Doorstop: Fatal", MB_OK | MB_ICONERROR);			\
		free(buff);																	\
		ExitProcess(EXIT_FAILURE);													\
	}

// A helper for cleaner error logging
#define ASSERT(test, message)                    \
	if(!(test))                                  \
	{                                            \
		MessageBox(NULL, message, L"Doorstop: Fatal", MB_OK | MB_ICONERROR); \
		ExitProcess(EXIT_FAILURE); \
	}

#define ASSERT_SOFT(test, ...)                   \
	if(!(test))                                  \
	{                                            \
		return __VA_ARGS__;                      \
	}
