#include "../config.h"
#include "../main.h"
#include "wincrt.h"
#include <windows.h>

BOOL WINAPI DllEntry(HINSTANCE hInstDll, DWORD reasonForDllLoad,
                     LPVOID reserved) {
    if (reasonForDllLoad == DLL_PROCESS_DETACH)
        SetEnvironmentVariableW(L"DOORSTOP_DISABLE", NULL);
    if (reasonForDllLoad != DLL_PROCESS_ATTACH)
        return TRUE;

    h_heap = GetProcessHeap();

    load_config();
    init_mono(NULL, NULL);
}