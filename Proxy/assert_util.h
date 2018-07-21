#pragma once


#ifdef _VERBOSE
#include <windows.h>
static HANDLE log_handle;
char buffer[4096];

inline void init_logger()
{
	log_handle = CreateFileA("doorstop.log", GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
	                         NULL);
}

inline void free_logger()
{
	CloseHandle(log_handle);
}

#define LOG(message, ...) \
	{ \
		size_t len = wsprintfA(buffer, message, __VA_ARGS__); \
		WriteFile(log_handle, buffer, len, NULL, NULL); \
	}
#else
inline void init_logger()
{
}

inline void free_logger()
{
}

#define LOG(message, ...) 
#endif

#define ASSERT_F(test, message, ...)												\
	if(!(test))																		\
	{																				\
		wchar_t *buff = (wchar_t*)memalloc(sizeof(wchar_t) * 1024);					\
		wsprintfW(buff, message, __VA_ARGS__);								\
		MessageBox(NULL, buff, L"Doorstop: Fatal", MB_OK | MB_ICONERROR);			\
		memfree(buff);																	\
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
