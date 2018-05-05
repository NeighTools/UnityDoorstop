/*
 * main.cpp -- The main "entry point" and the main logic of the DLL.
 *
 * Here, we define and initialize struct Main that contains the main code of this DLL.
 * 
 * The main procedure goes as follows:
 * 1. The loader checks that PatchLoader.dll and mono.dll exist
 * 2. mono.dll is loaded into memory and some of its functions are looked up
 * 3. mono_jit_init_version is hooked with the help of MinHook
 * 
 * Then, the loader waits until Unity creates its root domain for mono (which is done with mono_jit_init_version).
 * 
 * Inside mono_jit_init_version hook:
 * 1. Call the original mono_jit_init_version to get the Unity root domain
 * 2. Load PatchLoader.dll into the root domain
 * 3. Find and invoke PatchLoader.Loader.Run()
 * 
 * Rest of the work is done on the managed side.
 *
 */

#include <windows.h>
#include <string>
#include <atlstr.h>
#include <ctime>
#include <Psapi.h>
#include <Shlwapi.h>

#include "logger.h"

#define FINI_WIDE_SUPPORT

#define VERSION "2.0"
#define CONFIG_NAME L"doorstop_config.ini"
#define DEFAULT_TARGET_ASSEMBLY L"UnityDoorstop"

// A helper for cleaner error logging
#define ASSERT(test, message)                    \
	if(!(test))                                  \
	{                                            \
		Logger::fatalError(message);             \
	}

#define ASSERT_SOFT(test, ...)                   \
	if(!(test))                                  \
	{                                            \
		return __VA_ARGS__;                      \
	}

#include "mono.h"
#include "hook.h"

using namespace Mono;

bool enabled = true;
std::wstring targetAssembly = DEFAULT_TARGET_ASSEMBLY;

void loadConfig()
{
	wchar_t curPathProcess[MAX_PATH];
	GetModuleFileNameW(nullptr, curPathProcess, sizeof(curPathProcess));

	wchar_t curPath[_MAX_DIR];
	wchar_t drive[_MAX_DRIVE];
	_wsplitpath_s(curPathProcess, drive, sizeof(drive), curPath, sizeof(curPath), nullptr, 0, nullptr, 0);

	std::wstring iniPath;
	iniPath.append(drive).append(curPath).append(CONFIG_NAME);

	wchar_t enabledString[256] = L"true";
	GetPrivateProfileStringW(L"UnityDoorstop", L"enabled", L"true", enabledString, sizeof(enabledString),
	                         iniPath.c_str());

	if (_wcsnicmp(enabledString, L"true", 4) == 0)
		enabled = true;
	if (_wcsnicmp(enabledString, L"false", 5) == 0)
		enabled = false;

	wchar_t uppPathStr[MAX_PATH] = DEFAULT_TARGET_ASSEMBLY;
	GetPrivateProfileStringW(L"UnityDoorstop", L"targetAssembly", DEFAULT_TARGET_ASSEMBLY, uppPathStr, sizeof(uppPathStr),
	                         iniPath.c_str());

	targetAssembly = uppPathStr;
}

// The hook for mono_jit_init_version
// We use this since it will always be called once to initialize Mono's JIT
void* ownMonoJitInitVersion(const char* root_domain_name, const char* runtime_version)
{
	// Call the original mono_jit_init_version to initialize the Unity Root Domain
	const auto domain = mono_jit_init_version(root_domain_name, runtime_version);

	char dllPathA[MAX_PATH];
	sprintf_s(dllPathA, "%ws", targetAssembly.c_str());

	// Load our custom assembly into the domain
	const auto assembly = mono_domain_assembly_open(domain, dllPathA);
	ASSERT_SOFT(assembly != nullptr, domain);

	// Get assembly's image that contains CIL code
	const auto image = mono_assembly_get_image(assembly);
	ASSERT_SOFT(image != nullptr, domain);

	// Note: we use the runtime_invoke route since jit_exec will not work on DLLs

	// Create a descriptor for a random Main method
	const auto desc = mono_method_desc_new("*:Main", FALSE);
	ASSERT_SOFT(desc != nullptr, domain);

	// Find the first possible Main method in the assembly
	const auto method = mono_method_desc_search_in_image(desc, image);
	ASSERT_SOFT(method != nullptr, domain);

	const auto signature = mono_method_signature(method);
	ASSERT_SOFT(signature != nullptr, domain);

	// Get the number of parameters in the signature
	const auto params = mono_signature_get_param_count(signature);

	void** args = nullptr;
	if (params == 1)
	{
		// If there is a parameter, it's most likely a string[].
		// Poulate it as follows
		// 0 => path to the game's executable
		// 1 => --doorstop-invoke

		wchar_t path[MAX_PATH];
		GetModuleFileName(nullptr, path, sizeof(path));

		const auto exe_path = MONO_STRING(path);
		const auto doorstop_handle = MONO_STRING(L"--doorstop-invoke");

		const auto args_array = mono_array_new(domain, mono_get_string_class(), 2);

		SET_ARRAY_REF(args_array, 0, exe_path);
		SET_ARRAY_REF(args_array, 1, doorstop_handle);

		args = new void*[1];
		args[0] = args_array;
	}

	mono_runtime_invoke(method, nullptr, args, nullptr);

	if (args != nullptr)
	{
		delete[] args;
		args = nullptr;
	}

	return domain;
}


void Main()
{
	loadConfig();

	// If the loader is disabled, don't inject anything.
	if (!enabled)
		return;

	ASSERT_SOFT(PathFileExistsW(targetAssembly.c_str()));

	auto monoDllPath = getMonoPath();
	auto monoDllPathStr = monoDllPath.c_str();

	if (!PathFileExistsW(monoDllPathStr))
	{
		monoDllPath = getMonoPath(L"\\EmbedRuntime\\"); // Some versions of Unity have mono placed here instead!
		monoDllPathStr = monoDllPath.c_str();
		ASSERT(PathFileExistsW(monoDllPathStr), L"No mono.dll found! Cannot continue!");
	}

	// Preload mono into memory so we can start hooking it
	const auto monoLib = LoadLibrary(monoDllPathStr);

	ASSERT(monoLib != nullptr, L"Failed to load mono.dll!");

	loadMonoFunctions(monoLib);

	ezHook(monoLib, mono_jit_init_version, "winhttp.ownMonoJitInitVersion");
}

// ReSharper disable once CppInconsistentNaming
BOOL WINAPI DllMain(HINSTANCE /* hInstDll */, DWORD reasonForDllLoad, LPVOID /* reserved */)
{
	switch (reasonForDllLoad)
	{
	case DLL_PROCESS_ATTACH:
		Main();
		break;

	case DLL_PROCESS_DETACH:
		break;

	default:
		break;
	}

	return TRUE;
}
