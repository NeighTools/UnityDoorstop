#pragma once

#include <windows.h>
#include "crt.h"

inline wchar_t *widen(const char *str)
{
	size_t req_size = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
	wchar_t *result = malloc(req_size * sizeof(wchar_t));
	MultiByteToWideChar(CP_UTF8, 0, str, -1, result, req_size);
	return result;
}

inline char *narrow(const wchar_t *str)
{
	size_t req_size = WideCharToMultiByte(CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL);
	char *result = malloc(req_size * sizeof(char));
	WideCharToMultiByte(CP_UTF8, 0, str, -1, result, req_size, NULL, NULL);
	return result;
}

inline size_t get_module_path(HMODULE module, wchar_t **result, size_t *size, size_t free_space)
{
	size_t i = 0;
	size_t len, s;
	*result = NULL;
	do
	{
		if (*result != NULL)
			free(*result);
		i++;
		s = i * MAX_PATH + 1;
		*result = malloc(sizeof(wchar_t) * s);
		len = GetModuleFileNameW(module, *result, s);
	}
	while (GetLastError() == ERROR_INSUFFICIENT_BUFFER || s - len < free_space);

	if (size != NULL)
		*size = s;
	return len;
}

inline wchar_t *get_ini_entry(const wchar_t *config_file, const wchar_t *section, const wchar_t *key,
                              const wchar_t *default_val)
{
	size_t i = 0;
	size_t size, read;
	wchar_t *result = NULL;
	do
	{
		if (result != NULL)
			free(result);
		i++;
		size = i * MAX_PATH + 1;
		result = malloc(sizeof(wchar_t) * size);
		read = GetPrivateProfileStringW(section, key, default_val, result, size, config_file);
	}
	while (read == size - 1);
	return result;
}

inline wchar_t *get_folder_name(wchar_t *str, size_t len, BOOL with_separator)
{
	size_t i;
	for (i = len; i > 0; i--)
	{
		wchar_t c = *(str + i);
		if (c == L'\\' || c == L'/')
			break;
	}

	size_t result_len = i + (with_separator ? 1 : 0);
	wchar_t *result = calloc(result_len + 1, sizeof(wchar_t));
	wmemcpy(result, str, result_len);
	return result;
}

inline wchar_t *get_file_name_no_ext(wchar_t *str, size_t len)
{
	size_t ext_index = len;
	size_t i;
	for (i = len; i > 0; i--)
	{
		wchar_t c = *(str + i);
		if (c == L'.' && ext_index == len)
			ext_index = i;
		else if (c == L'\\' || c == L'/')
			break;
	}

	size_t result_len = ext_index - i;
	wchar_t *result = calloc(result_len, sizeof(wchar_t));
	wmemcpy(result, str + i + 1, result_len - 1);
	return result;
}


inline wchar_t *get_full_path(wchar_t *str, size_t len)
{
	size_t needed = GetFullPathNameW(str, 0, NULL, NULL);
	wchar_t *res = malloc(sizeof(wchar_t) * needed);
	GetFullPathNameW(str, needed, res, NULL);
	return res;
}

inline size_t get_working_dir(wchar_t **result)
{
	size_t len = GetCurrentDirectoryW(0, NULL);
	*result = malloc(sizeof(wchar_t) * len);
	GetCurrentDirectoryW(len, *result);
	return len;
}
