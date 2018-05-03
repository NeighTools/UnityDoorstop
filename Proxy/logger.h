#pragma once
#pragma warning(disable: 4505) //Unreferenced local function has been removed

#include <windows.h>
#include <fstream>
#include <string>
#include <cstdlib>

#define LOG(message) Logger::getLogStream() << message << std::endl

namespace Logger
{
	DECLSPEC_NORETURN static void fatalError(const std::wstring& message)
	{
		MessageBox(nullptr, message.c_str(), L"Proxy Fatal Error", MB_OK | MB_ICONERROR);

		std::exit(EXIT_FAILURE);
	}
}
