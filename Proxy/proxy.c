/* ==================================
 * COMPUTER GENERATED -- DO NOT EDIT
 * ==================================
 * 
 * This file contains the definitions for all proxy functions this DLL supports.
 * 
 * The proxies are very simple functions that should be optimizied into a single JMP instruction without editing the stack at all.
 */

#include <windows.h>

#define ADD_ORIGINAL(i, name) originalFunctions[i] = GetProcAddress(dll, #name)

#define PROXY(call, name, i) \
	ULONG call name() \
	{ \
		return originalFunctions[i](); \
	}

FARPROC originalFunctions[1] = {0};

void loadFunctions(HMODULE dll)
{
	ADD_ORIGINAL(0, WinHttpGetIEProxyConfigForCurrentUser);
}

PROXY(__stdcall, WinHttpGetIEProxyConfigForCurrentUser, 0);