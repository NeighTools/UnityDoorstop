#include "../config.h"
#include "../logging.h"
#include "../main.h"
#include "wincrt.h"
#include <windows.h>

/**
 * @brief Ensures current working directory is the game folder
 *
 * In some cases (e.g. custom launchers), the CWD (current working directory)
 * is not the same as Unity default. This can break invokable DLLs and even
 * Unity itself. This fix ensures current working directory is the same as
 * application directory and fixes it if not.
 *
 * @return bool_t Whether CWD was changed to match program directory.
 */
bool_t fix_cwd() {
    char_t *app_path = program_path();
    char_t *app_dir = get_folder_name(app_path);
    bool_t fixed_cwd = FALSE;
    char_t *working_dir = get_working_dir();

    if (strcmpi(app_dir, working_dir) != 0) {
        fixed_cwd = TRUE;
        SetCurrentDirectory(app_dir);
    }

    free(app_path);
    free(app_dir);
    free(working_dir);
    return fixed_cwd;
}

BOOL WINAPI DllEntry(HINSTANCE hInstDll, DWORD reasonForDllLoad,
                     LPVOID reserved) {
    if (reasonForDllLoad == DLL_PROCESS_DETACH)
        SetEnvironmentVariableW(L"DOORSTOP_DISABLE", NULL);
    if (reasonForDllLoad != DLL_PROCESS_ATTACH)
        return TRUE;

    h_heap = GetProcessHeap();
    load_config();
    bool_t fixed_cwd = fix_cwd();
    start_logger(hInstDll, fixed_cwd);
}