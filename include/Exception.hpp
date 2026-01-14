/*
	* LilyLib::Exception
	* Exception class

	* Copyright (c) 2026 Lily

	* This program is free software; licensed under the MIT license.
	* You should have received a copy of the license along with this program.
	* If not, see <https://opensource.org/licenses/MIT>.
*/
#pragma once
#ifndef LILY_LIB_EXCEPTION_H
#define LILY_LIB_EXCEPTION_H
#include <format>
#include <string>
#include <stdexcept>
#include <source_location>
namespace LilyLib {
	class DetailedException : public std::runtime_error {
	public:
		template <typename... Args>
		explicit DetailedException(const std::source_location loc,
			std::format_string<Args...> format_str,
			Args&&... args)
			: std::runtime_error(format_error(loc, std::format(format_str, std::forward<Args>(args)...))) {}
		
		explicit DetailedException(const std::source_location loc,
			const std::string &err)
			: std::runtime_error(format_error(loc, err)) {}

		static void ComposeTrace(const std::exception &e, std::string &out, int level = 0);
	private:
		static std::string format_error(const std::source_location &loc, const std::string& msg);
	};
}
#endif