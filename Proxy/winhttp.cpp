#include <windows.h>
#include <string>
#include <vector>
#include <cstdint>
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

namespace ProxyDll
{
	static std::wstring ptrToString(const void* ptr)
	{
		wchar_t tempString[128];

#if defined(_M_IX86)
		_snwprintf_s(tempString, sizeof(tempString), L"0x%08X",
		             reinterpret_cast<std::uintptr_t>(ptr));
#elif defined(_M_X64)
		_snwprintf_s(tempString, sizeof(tempString), L"0x%016llX",
		             reinterpret_cast<std::uintptr_t>(ptr));
#endif // x86/64

		return tempString;
	}

	static std::wstring lastWinErrorAsString()
	{
		// Adapted from this SO thread:
		// http://stackoverflow.com/a/17387176/1198654

		auto errorMessageID = GetLastError();
		if (errorMessageID == 0)
		{
			return L"Unknown error";
		}

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

	static std::wstring getRealDllPath()
	{
		// Try to look for the alternative opengl32 first in the same directory.
		if (PathFileExists(ALT_NAME))
			return ALT_NAME;

		wchar_t systemDir[1024] = {'\0'};
		GetSystemDirectory(systemDir, sizeof(systemDir));

		std::wstring result;
		if (systemDir[0] != '\0')
		{
			result += systemDir;
			result += L"\\";
			result += PROXY_NAME;
		}
		else // Something wrong... Try a hardcoded path...
		{
			result = WIN_PATH;
			result += PROXY_NAME;
		}
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

	struct RealDll
	{
		HMODULE dllHandle;
		std::wstring dllFilePath;

		RealDll(const RealDll&) = delete;
		RealDll& operator =(const RealDll&) = delete;

		RealDll()
		{
			load();
		}

		~RealDll()
		{
			unload();
		}

		void load()
		{
			if (isLoaded())
			{
				Logger::fatalError(L"The Real DLL is already loaded!");
			}

			const auto originalDllPath = getRealDllPath();
			//LOG(L"Trying to load real DLL from \"" << originalDllPath << L"\"...");

			dllHandle = LoadLibraryEx(originalDllPath.c_str(), nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
			if (dllHandle == nullptr)
			{
				Logger::fatalError(L"Unable to load the real DLL!\n" + lastWinErrorAsString());
			}

			const auto selfHMod = getSelfModuleHandle();
			if (dllHandle == selfHMod)
			{
				Logger::fatalError(L"Trying to load itself as the real DLL!");
			}

			wchar_t tempString[1024] = {'\0'};
			if (GetModuleFileName(dllHandle, tempString, sizeof(tempString)) == 0)
			{
				LOG(L"Unable to get Real DLL file path!");
			}
			else
			{
				dllFilePath = tempString;
			}

			/*LOG(L"\n--------------------------------------------------------");
			LOG(L"  Real DLL is loaded!");
			LOG(L"  Real DLL = " << ptrToString(dllHandle) << L", Proxy DLL = " << ptrToString(selfHMod));
			LOG(L"  Real DLL path: \"" << dllFilePath << L"\"");
			LOG(L"--------------------------------------------------------\n");*/
		}

		void unload()
		{
			if (isLoaded())
			{
				FreeLibrary(dllHandle);
				dllHandle = nullptr;
				dllFilePath.clear();
			}
		}

		bool isLoaded() const
		{
			return dllHandle != nullptr;
		}

		void* getFuncPtr(const char* funcName) const
		{
			if (!isLoaded())
			{
				LOG("Error! Real DLL not loaded. Can't get function " << funcName);
				return nullptr;
			}

			const auto fptr = GetProcAddress(dllHandle, funcName);
			if (fptr == nullptr)
			{
				LOG("Error! Unable to find " << funcName);
			}

			return reinterpret_cast<void *>(fptr);
		}

		// Just one instance per process.
		// Also only attempt to load the DLL on the first reference.
		static RealDll& getInstance()
		{
			static RealDll glDll;
			return glDll;
		}
	};

	static void* getRealDllFunction(const char* funcName)
	{
		auto& glDll = RealDll::getInstance();
		auto addr = glDll.getFuncPtr(funcName);
		//LOG("Loading real func: (" << ptrToString(addr) << ") " << funcName);
		return addr;
	}
}


// We define the struct here, since we don't wont to include any unneeded clutter

typedef struct
{
	BOOL fAutoDetect;
	LPWSTR lpszAutoConfigUrl;
	LPWSTR lpszProxy;
	LPWSTR lpszProxyBypass;
} WINHTTP_CURRENT_USER_IE_PROXY_CONFIG;


typedef BOOL (WINAPI * Func)(_Inout_ WINHTTP_CURRENT_USER_IE_PROXY_CONFIG*);

extern "C" BOOL WINAPI WinHttpGetIEProxyConfigForCurrentUser(_Inout_ WINHTTP_CURRENT_USER_IE_PROXY_CONFIG* pProxyConfig)
{
	static auto original = reinterpret_cast<Func>(ProxyDll::getRealDllFunction("WinHttpGetIEProxyConfigForCurrentUser"));
	return original(pProxyConfig);
}
