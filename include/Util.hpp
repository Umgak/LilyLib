/*
	* LilyLib::Util
	* Contains some logging/helper functions.

	* Copyright (c) 2026 Lily
 
	* This program is free software; licensed under the MIT license.
	* You should have received a copy of the license along with this program.
	* If not, see <https://opensource.org/licenses/MIT>.
*/
#pragma once
#ifndef LILY_LIB_UTIL_H
#define LILY_LIB_UTIL_H
#include <filesystem>
#include <fstream>
#include <format>

namespace LilyLib::Util {
	extern std::ofstream uLogFile;

	// Get names
	std::string GetCurrentProcessName();
	std::filesystem::path GetCurrentModPath();
	std::string GetCurrentModName();
	
	// Wait
	void WaitForProcess();

	// Logging
	void OpenModLogFile();
	void CloseLog();
	void Log(std::string_view message);

	template<typename... Args>
	inline void Log(std::format_string<Args...> format_str, Args&&... args)
	{
		Log(std::format(format_str, std::forward<Args>(args)...));
	}

	void ShowErrorMessage(std::string err);

	template<typename... Args>
	inline void ShowErrorMessage(std::format_string<Args...> format_str, Args&&... args)
	{
		ShowErrorMessage(std::format(format_str, std::forward<Args>(args)...));
	}
}
#endif