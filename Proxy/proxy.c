/* ==================================
 * COMPUTER GENERATED -- DO NOT EDIT
 * ==================================
 */

#include <windows.h>

extern FARPROC __GetFileVersionInfoA__;
extern FARPROC __GetFileVersionInfoByHandle__;
extern FARPROC __GetFileVersionInfoExA__;
extern FARPROC __GetFileVersionInfoExW__;
extern FARPROC __GetFileVersionInfoSizeA__;
extern FARPROC __GetFileVersionInfoSizeExA__;
extern FARPROC __GetFileVersionInfoSizeExW__;
extern FARPROC __GetFileVersionInfoSizeW__;
extern FARPROC __GetFileVersionInfoW__;
extern FARPROC __VerFindFileA__;
extern FARPROC __VerFindFileW__;
extern FARPROC __VerInstallFileA__;
extern FARPROC __VerInstallFileW__;
extern FARPROC __VerLanguageNameA__;
extern FARPROC __VerLanguageNameW__;
extern FARPROC __VerQueryValueA__;
extern FARPROC __VerQueryValueW__;


void loadFunctions(HMODULE dll)
{
__GetFileVersionInfoA__ = GetProcAddress(dll, "GetFileVersionInfoA");
__GetFileVersionInfoByHandle__ = GetProcAddress(dll, "GetFileVersionInfoByHandle");
__GetFileVersionInfoExA__ = GetProcAddress(dll, "GetFileVersionInfoExA");
__GetFileVersionInfoExW__ = GetProcAddress(dll, "GetFileVersionInfoExW");
__GetFileVersionInfoSizeA__ = GetProcAddress(dll, "GetFileVersionInfoSizeA");
__GetFileVersionInfoSizeExA__ = GetProcAddress(dll, "GetFileVersionInfoSizeExA");
__GetFileVersionInfoSizeExW__ = GetProcAddress(dll, "GetFileVersionInfoSizeExW");
__GetFileVersionInfoSizeW__ = GetProcAddress(dll, "GetFileVersionInfoSizeW");
__GetFileVersionInfoW__ = GetProcAddress(dll, "GetFileVersionInfoW");
__VerFindFileA__ = GetProcAddress(dll, "VerFindFileA");
__VerFindFileW__ = GetProcAddress(dll, "VerFindFileW");
__VerInstallFileA__ = GetProcAddress(dll, "VerInstallFileA");
__VerInstallFileW__ = GetProcAddress(dll, "VerInstallFileW");
__VerLanguageNameA__ = GetProcAddress(dll, "VerLanguageNameA");
__VerLanguageNameW__ = GetProcAddress(dll, "VerLanguageNameW");
__VerQueryValueA__ = GetProcAddress(dll, "VerQueryValueA");
__VerQueryValueW__ = GetProcAddress(dll, "VerQueryValueW");

}
