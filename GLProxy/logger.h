#pragma once
#pragma warning(disable: 4505) //Unreferenced local function has been removed

#include <windows.h>
#include <fstream>
#include <string>
#include <cstdlib>

#define LOG(message) Logger::getLogStream() << message << std::endl

namespace Logger
{
	static std::wstring getTimeString()
	{
		const auto rawTime = std::time(nullptr);
		std::wstring ts = _wctime(&rawTime);
		ts.pop_back(); // Remove the default '\n' added by ctime.
		return ts;
	}

	static std::wofstream& getLogStream()
	{
		static std::wofstream log("opengl_proxy.log");
		return log;
	}

	DECLSPEC_NORETURN static void fatalError(const std::wstring& message)
	{
		MessageBox(nullptr, message.c_str(), L"GLProxy Fatal Error", MB_OK | MB_ICONERROR);

		LOG("Fatal error: " << message);
		getLogStream().flush();

		std::exit(EXIT_FAILURE);
	}
}
