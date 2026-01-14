/*
	* LilyLib::Exception
	* Exception class

	* Copyright (c) 2026 Lily

	* This program is free software; licensed under the MIT license.
	* You should have received a copy of the license along with this program.
	* If not, see <https://opensource.org/licenses/MIT>.
*/
#include "../include/Exception.hpp"
#include <filesystem>
namespace LilyLib {
	void DetailedException::ComposeTrace(const std::exception &e, std::string &out, int level)
	{
		if (level > 0) out += "\n\n--- Caused by ---";
		out += e.what();
		
		try {
			std::rethrow_if_nested(e);
		} catch (const std::exception &nested) {
			ComposeTrace(nested, out, level + 1);
		} catch (...){}
	}

	std::string DetailedException::format_error(const std::source_location &loc, const std::string& err)
	{
		// Don't want the entire fucking function def
		std::string func = loc.function_name();
		size_t paren = func.find_first_of('(');
		if (paren != std::string::npos) func.erase(paren, func.length());
		size_t last_space = func.find_last_of(' ');
		if (last_space != std::string::npos) func.erase(0, last_space + 1);

		// Don't want the entire god damn filepath either
		std::filesystem::path p = loc.file_name();
		std::string relative_path = std::filesystem::relative(p, PROJECT_ROOT).string();


		return std::format(R"(
{}
  in function {}
  at {}:{})", err, func, relative_path, loc.line());
	}
}