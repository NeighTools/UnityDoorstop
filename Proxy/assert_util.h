#pragma once

#include <windows.h>

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
