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

#include <windows.h>
#include <stdio.h>
#include <Shlwapi.h>
#include "assert_util.h"

#define WIN_PATH L"C:\\windows\\system32\\"

extern FARPROC originalFunctions[];
extern void loadFunctions(HMODULE dll);

// Load the proxy functions into memory
inline void loadProxy(wchar_t *moduleName)
{
	wchar_t altName[_MAX_FNAME + 1]; // We also define an _alt file to look for
	wchar_t dllPath[MAX_PATH + 1];	// The final DLL path
	
	swprintf_s(altName, _MAX_FNAME + 1, L"%s_alt.dll", moduleName);
	const int altFullPathSize = GetFullPathName(altName, 0, NULL, NULL);
	wchar_t *altFullPath = malloc(sizeof(wchar_t) * altFullPathSize);
	GetFullPathName(altName, altFullPathSize, altFullPath, NULL);

	// Try to look for the alternative first in the same directory.
	HMODULE handle = LoadLibrary(altFullPath);

	if(handle == NULL)
	{
		wchar_t systemDir[MAX_PATH + 1] = L"\0";
		GetSystemDirectory(systemDir, MAX_PATH + 1);
		if (systemDir[0] != '\0')
			swprintf_s(dllPath, MAX_PATH + 1, L"%s\\%s.dll", systemDir, moduleName);
		else
			swprintf_s(dllPath, MAX_PATH + 1, L"%s%s.dll", WIN_PATH, moduleName);

		handle = LoadLibrary(dllPath);
	}
	
	ASSERT_F(handle != NULL, L"Unable to load the original %s.dll!\n\nTried to look for these:\n%s\n%s", moduleName, altFullPath, dllPath);

	free(altFullPath);

	loadFunctions(handle);
}