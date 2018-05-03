#include <windows.h>
#include <string>
#include <vector>
#include <ctime>
#include <Psapi.h>
#include <Shlwapi.h>
#include <atlstr.h>

#include "logger.h"

#define WINAPI __stdcall

// ========================================================
// Utilities for DLL proxying
// ========================================================

#define PROXY_NAME L"winhttp.dll"
#define ALT_NAME L"winhttp_alt.dll"
#define WIN_PATH L"C:\\windows\\system32\\"

namespace WinHttp
{
	HMODULE dllHandle;
	std::wstring dllFilePath;

	static std::wstring getRealDllPath()
	{
		// Try to look for the alternative opengl32 first in the same directory.
		if (PathFileExists(ALT_NAME))
			return ALT_NAME;

		wchar_t systemDir[1024] = {'\0'};
		GetSystemDirectory(systemDir, sizeof(systemDir));

		std::wstring result;

		if (systemDir[0] != '\0')
			result.append(systemDir).append(L"\\").append(PROXY_NAME);
		else
			result.append(WIN_PATH).append(PROXY_NAME); // Something wrong... Try a hardcoded path...

		return result;
	}

	static HMODULE getSelfModuleHandle()
	{
		//
		// This is somewhat hackish, but should work.
		// We try to get this module's address from the address
		// of one of its functions, this very function actually.
		// Worst case it fails and we return null.
		//
		// There's also the '__ImageBase' hack, but that seems even more precarious...
		// http://stackoverflow.com/a/6924293/1198654
		//
		HMODULE selfHMod = nullptr;
		GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
		                  GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		                  (LPCWSTR)&getSelfModuleHandle,
		                  &selfHMod);
		return selfHMod;
	}

	static std::wstring lastWinErrorAsString()
	{
		// Adapted from this SO thread:
		// http://stackoverflow.com/a/17387176/1198654

		const auto errorMessageID = GetLastError();
		if (errorMessageID == 0)
			return L"Unknown error";

		LPWSTR messageBuffer = nullptr;
		constexpr DWORD fmtFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS;

		const auto size = FormatMessage(fmtFlags, nullptr, errorMessageID,
		                                MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT),
		                                (LPWSTR)&messageBuffer, 0, nullptr);

		const std::wstring message{messageBuffer, size};
		LocalFree(messageBuffer);

		return message + L"(error " + std::to_wstring(errorMessageID) + L")";
	}

	void load()
	{
		const auto originalDllPath = getRealDllPath();
		
		dllHandle = LoadLibraryEx(originalDllPath.c_str(), nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);

		if (dllHandle == nullptr)
			Logger::fatalError(L"Unable to load the real DLL!\n" + lastWinErrorAsString());

		const auto selfHMod = getSelfModuleHandle();

		if (dllHandle == selfHMod)
			Logger::fatalError(L"Trying to load itself as the real DLL!");

		wchar_t tempString[1024] = {'\0'};

		if (GetModuleFileName(dllHandle, tempString, sizeof(tempString)) == 0)
			Logger::fatalError(L"Unable to get Real DLL file path!");
		else
			dllFilePath = tempString;
	}

	void* getRealDllFunction(const char* funcName)
	{
		if (dllHandle == nullptr)
			load();

		const auto fptr = GetProcAddress(dllHandle, funcName);

		return reinterpret_cast<void *>(fptr);
	}
}


// The "original" function we export
// Since we don't care about the contents of the struct, we simply use void *
typedef BOOL (WINAPI * Func)(_Inout_ void*);

extern "C" BOOL WINAPI WinHttpGetIEProxyConfigForCurrentUser(_Inout_ void* pProxyConfig)
{
	static auto original = reinterpret_cast<Func>(WinHttp::getRealDllFunction("WinHttpGetIEProxyConfigForCurrentUser"));
	return original(pProxyConfig);
}
