#pragma once

#include <windows.h>

#define CONFIG_NAME L"doorstop_config.ini"
#define DEFAULT_TARGET_ASSEMBLY L"Doorstop.dll"
#define EXE_EXTENSION_LENGTH 4

struct {
    BOOL enabled;
    BOOL redirect_output_log;
    BOOL ignore_disabled_env;
    wchar_t *target_assembly;
    wchar_t *mono_lib_dir;
    wchar_t *mono_config_dir;
    wchar_t *mono_corlib_dir;
    wchar_t *mono_bcl_root_dir;
} config;


void load_config();
