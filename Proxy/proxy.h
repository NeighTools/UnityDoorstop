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

EXTERN_C IMAGE_DOS_HEADER __ImageBase; // This is provided by MSVC with the infomration about this DLL

// Load the proxy functions into memory
inline void loadProxy()
{
	// Resolve the path of this DLL
	wchar_t path[MAX_PATH + 1];
	GetModuleFileName((HINSTANCE)&__ImageBase, path, MAX_PATH + 1);

	// Resolve the name of the DLL
	wchar_t dllName[_MAX_FNAME + 1];
	_wsplitpath_s(path, NULL, 0, NULL, 0, dllName, _MAX_FNAME + 1, NULL, 0);

	wchar_t altName[_MAX_FNAME + 1]; // We also define an _alt file to look for
	swprintf_s(altName, _MAX_FNAME + 1, L"%s_alt.dll", dllName);

	wchar_t dllPath[MAX_PATH + 1];	// The final DLL path

	// Try to look for the alternative first in the same directory.
	if (PathFileExists(altName))
		wcscpy_s(dllPath, MAX_PATH + 1, altName);
	else // Otherwise look from Windows dir
	{
		wchar_t systemDir[MAX_PATH + 1] = {'\0'};
		GetSystemDirectory(systemDir, sizeof(systemDir));

		if (systemDir[0] != '\0')
			swprintf_s(dllPath, MAX_PATH + 1, L"%s\\%s.dll", systemDir, dllName);
		else
			swprintf_s(dllPath, MAX_PATH + 1, L"%s%s.dll", WIN_PATH, dllName);
	}

	HMODULE dllHandle = LoadLibraryEx(dllPath, NULL, LOAD_LIBRARY_SEARCH_SYSTEM32 | LOAD_LIBRARY_SEARCH_APPLICATION_DIR);
	ASSERT(dllHandle != NULL, L"Unable to load the original DLL!");

	loadFunctions(dllHandle);
}

#define PROXY(call, name, i) \
	ULONG call name() \
	{ \
		return originalFunctions[i](); \
	}
