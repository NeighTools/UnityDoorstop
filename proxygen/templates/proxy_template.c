/* ==================================
 * COMPUTER GENERATED -- DO NOT EDIT
 * ==================================
 * 
 * This file contains the definitions for all proxy functions this DLL supports.
 * 
 * The proxies are very simple functions that should be optimizied into a 
 * single JMP instruction without editing the stack at all.
 * 
 * NOTE: While this works, this is a somewhat hackish approach that is based on how 
 * the compiler optimizes the code. That said, the proxy will not work on Debug build currently
 * (that can be fixed by changing the appropriate compile flag that I am yet to locate).
 */

#include <windows.h>

#define ADD_ORIGINAL(i, name) originalFunctions[i] = GetProcAddress(dll, #name)

#define PROXY(i, name) \
	ULONG name() \
	{ \
		return originalFunctions[i](); \
	}

FARPROC originalFunctions[${proxy_count}] = {0};

void loadFunctions(HMODULE dll)
{
${proxy_add}
}

${proxy_def}