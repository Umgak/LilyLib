/*
	* LilyLib::Util
	* Contains some logging/helper functions.

	* Copyright (c) 2026 Lily

	* This program is free software; licensed under the MIT license.
	* You should have received a copy of the license along with this program.
	* If not, see <https://opensource.org/licenses/MIT>.
*/
#include "../include/Util.hpp"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif
namespace LilyLib::Util {
	std::ofstream uLogFile;

	static void dummy() {}

	static std::filesystem::path _GetModulePath(bool mainProcessModule) {
		HMODULE module = nullptr;
		char lpFilepath[MAX_PATH] = {};

		if (!mainProcessModule)
		{
			GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)&dummy, &module);
		}
		GetModuleFileNameA(module, lpFilepath, sizeof(lpFilepath));
		return std::filesystem::path(lpFilepath);
	}

	static std::string _GetModuleName(bool mainProcessModule)
	{
		return _GetModulePath(mainProcessModule).filename().string();
	}

	std::string GetCurrentProcessName()
	{
		return _GetModuleName(true);
	}

	std::filesystem::path GetCurrentModPath()
	{
		return _GetModulePath(false);
	}

	std::string GetCurrentModName()
	{
		return GetCurrentModPath().stem().string();
	}

	void WaitForProcess()
	{
		WaitForInputIdle(GetCurrentProcess(), INFINITE);
	}

	void OpenModLogFile()
	{
		if (!uLogFile.is_open())
		{
			uLogFile.open(GetCurrentModPath().replace_extension(".log"));
		}
	}

	void CloseLog()
	{
		if (uLogFile.is_open())
		{
			uLogFile.close();
		}
	}

	void Log(std::string_view message)
	{
		OpenModLogFile();
		const auto now = std::chrono::system_clock::now();
		std::string logLine = std::format("{:%T}: {}: {}\n", now, GetCurrentModName(), message);

		if (uLogFile.is_open())
		{
			uLogFile << logLine;
			uLogFile.flush();
		}
	}

	void ShowErrorMessage(std::string err)
	{
		MessageBoxA(NULL, err.c_str(), GetCurrentModName().c_str(), MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);
	}
}