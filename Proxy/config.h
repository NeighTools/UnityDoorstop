#pragma once

#include <windows.h>
#include <shellapi.h>
#include "winapi_util.h"
#include "assert_util.h"

#define CONFIG_NAME L"doorstop_config.ini"
#define DEFAULT_TARGET_ASSEMBLY L"Doorstop.dll"
#define EXE_EXTENSION_LENGTH 4

BOOL enabled = FALSE;
wchar_t *target_assembly = NULL;

#define STR_EQUAL(str1, str2) (lstrcmpiW(str1, str2) == 0)

inline void init_config_file()
{
	if (GetFileAttributesW(CONFIG_NAME) == INVALID_FILE_ATTRIBUTES)
		return;

	const size_t len = GetFullPathNameW(CONFIG_NAME, 0, NULL, NULL);
	wchar_t *configPath = memalloc(sizeof(wchar_t) * len);
	GetFullPathNameW(CONFIG_NAME, len, configPath, NULL);

	wchar_t enabledString[256] = L"true";
	GetPrivateProfileStringW(L"UnityDoorstop", L"enabled", L"true", enabledString, 256, configPath);

	if (STR_EQUAL(enabledString, L"true"))
		enabled = TRUE;
	else if (STR_EQUAL(enabledString, L"false"))
		enabled = FALSE;

	target_assembly = get_ini_entry(configPath, L"UnityDoorstop", L"targetAssembly", DEFAULT_TARGET_ASSEMBLY);

	LOG("Config; Target assembly: %S\n", target_assembly);

	memfree(configPath);
}

inline void init_cmd_args()
{
	wchar_t *args = GetCommandLineW();
	int argc = 0;
	wchar_t **argv = CommandLineToArgvW(args, &argc);

#define IS_ARGUMENT(arg_name) STR_EQUAL(arg, arg_name) && i < argc

	for (int i = 0; i < argc; i++)
	{
		wchar_t *arg = argv[i];
		if (IS_ARGUMENT(L"--doorstop-enable"))
		{
			wchar_t *par = argv[++i];

			if (STR_EQUAL(par, L"true"))
				enabled = TRUE;
			else if (STR_EQUAL(par, L"false"))
				enabled = FALSE;
		}
		else if (IS_ARGUMENT(L"--doorstop-target"))
		{
			if (target_assembly != NULL)
				memfree(target_assembly);
			const size_t len = wcslen(argv[i + 1]) + 1;
			target_assembly = memalloc(sizeof(wchar_t) * len);
			lstrcpynW(target_assembly, argv[++i], len);
			LOG("Args; Target assembly: %S\n", target_assembly);
		}
	}

	LocalFree(argv);
}

inline void init_env_vars()
{
	if (GetEnvironmentVariableW(L"DOORSTOP_DISABLE", NULL, 0) != 0)
	{
		LOG("DOORSTOP_DISABLE is set! Disabling Doorstop!\n");
		enabled = FALSE;
	}
}

inline void load_config()
{
	init_config_file();
	init_cmd_args();
	init_env_vars();
}

inline void cleanup_config()
{
	if (target_assembly != NULL)
		memfree(target_assembly);
}
