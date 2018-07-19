/*
 * Proxy.h -- Definitions for proxy-related functionality
 * 
 * The proxy works roughly as follows:
 * - We define our exports in proxy.c (computer generated)
 * - loadProxy initializes the proxy:
 *     1. Look up the name of this DLL
 *     2. Find the original DLL with the same name
 *     3. Load the original DLL
 *     4. Load all functions into originalFunctions array
 *     
 * For more information, refer to proxy.c 
 */

#pragma once

#include "ver.h"
#include <windows.h>
#include <stdio.h>
#include <Shlwapi.h>
#include "assert_util.h"

#define WIN_PATH L"C:\\windows\\system32\\"

extern FARPROC originalFunctions[];
extern void loadFunctions(HMODULE dll);

// Load the proxy functions into memory
inline void loadProxy(wchar_t* moduleName)
{
	size_t alt_name_len = wcslen(moduleName) + 10;
	wchar_t* alt_name = malloc(sizeof(wchar_t) * alt_name_len);
	swprintf_s(alt_name, alt_name_len, L"%s_alt.dll", moduleName);

	wchar_t* dll_path = NULL; // The final DLL path

	const int alt_full_path_len = GetFullPathName(alt_name, 0, NULL, NULL);
	wchar_t* alt_full_path = malloc(sizeof(wchar_t) * alt_full_path_len);
	GetFullPathName(alt_name, alt_full_path_len, alt_full_path, NULL);
	free(alt_name);

	// Try to look for the alternative first in the same directory.
	HMODULE handle = LoadLibrary(alt_full_path);

	if (handle == NULL)
	{
		size_t system_dir_len = GetSystemDirectory(NULL, 0);
		dll_path = malloc(sizeof(wchar_t) * (system_dir_len + alt_name_len));
		GetSystemDirectory(dll_path, system_dir_len);
		swprintf_s(dll_path + system_dir_len - 1, alt_name_len, L"\\%s.dll", moduleName);
		handle = LoadLibrary(dll_path);
	}

	ASSERT_F(handle != NULL, L"Unable to load the original %s.dll!\n\nTried to look for these:\n%s\n%s", moduleName,
		alt_full_path, dll_path);

	free(alt_full_path);
	free(dll_path);

	loadFunctions(handle);
}
