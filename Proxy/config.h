#pragma once

#include <windows.h>
#include <stdio.h>
#include <Shlwapi.h>
#include <wchar.h>

#define CONFIG_NAME L"doorstop_config.ini"
#define DEFAULT_TARGET_ASSEMBLY L"Doorstop.dll"
#define EXE_EXTENSION_LENGTH 4

BOOL enabled = FALSE;
wchar_t targetAssembly[MAX_PATH + 1];
wchar_t monoDllFallback[MAX_PATH + 1];

#define STR_EQUAL(str1, str2) (_wcsnicmp(str1, str2, wcslen(str2)) == 0)

inline void initConfigFile()
{
	wchar_t curPathProcess[MAX_PATH + 1];
	GetModuleFileNameW(NULL, curPathProcess, sizeof(curPathProcess));

	wchar_t curPath[_MAX_DIR + 1];
	wchar_t drive[_MAX_DRIVE + 1];

	_wsplitpath_s(curPathProcess, drive, _MAX_DRIVE + 1, curPath, _MAX_DIR + 1, NULL, 0, NULL, 0);

	wchar_t iniPath[MAX_PATH + 1];
	swprintf_s(iniPath, MAX_PATH + 1, L"%s%s%s", drive, curPath, CONFIG_NAME);

	wchar_t enabledString[256] = L"true";
	GetPrivateProfileStringW(L"UnityDoorstop", L"enabled", L"true", enabledString, 256, iniPath);

	if (STR_EQUAL(enabledString, L"true"))
		enabled = TRUE;
	else if (STR_EQUAL(enabledString, L"false"))
		enabled = FALSE;

	wchar_t uppPathStr[MAX_PATH + 1] = DEFAULT_TARGET_ASSEMBLY;
	GetPrivateProfileStringW(L"UnityDoorstop", L"targetAssembly", DEFAULT_TARGET_ASSEMBLY, uppPathStr, MAX_PATH + 1,
	                         iniPath);

	GetPrivateProfileStringW(L"UnityDoorstop", L"monoFallback", L"\0", monoDllFallback, MAX_PATH + 1,
	                         iniPath);

	wcscpy_s(targetAssembly, MAX_PATH + 1, uppPathStr);
}

inline void initCmdArgs()
{
	wchar_t* args = GetCommandLineW();
	int argc = 0;
	wchar_t** argv = CommandLineToArgvW(args, &argc);

#define IS_ARGUMENT(arg_name) STR_EQUAL(arg, arg_name) && i < argc

	for (int i = 0; i < argc; i++)
	{
		wchar_t* arg = argv[i];
		if (IS_ARGUMENT(L"--doorstop-enable"))
		{
			wchar_t* par = argv[++i];

			if (STR_EQUAL(par, L"true"))
				enabled = TRUE;
			else if (STR_EQUAL(par, L"false"))
				enabled = FALSE;
		}
		else if (IS_ARGUMENT(L"--doorstop-target"))
		{
			wmemset(targetAssembly, L"\0", MAX_PATH + 1);
			wcscpy_s(targetAssembly, MAX_PATH + 1, argv[++i]);
		}
		else if (IS_ARGUMENT(L"--doorstop-mono-fallback"))
		{
			wmemset(monoDllFallback, L"\0", MAX_PATH + 1);
			wcscpy_s(monoDllFallback, MAX_PATH + 1, argv[++i]);
		}
	}
}

inline void initEnvVars()
{
	if (GetEnvironmentVariableW(L"DOORSTOP_DISABLE", NULL, 0) != 0)
		enabled = FALSE;
}

inline void loadConfig()
{
	initConfigFile();
	initCmdArgs();
	initEnvVars();
}
