/*
	* LilyLib::Memory
	* Functions for scanning memory
	* Requires Pattern16

	* Copyright (c) 2026 Lily

	* This program is free software; licensed under the MIT license.
	* You should have received a copy of the license along with this program.
	* If not, see <https://opensource.org/licenses/MIT>.
*/

#pragma once
#ifndef LILY_LIB_MEMORY_H
#define LILY_LIB_MEMORY_H
#include <string>
#include <cstddef>

namespace LilyLib::Memory {
	// Use Pattern16 to scan memory (by default .text section) for an AOB
	void* AOBScanModule(const std::string& aob, const char* const module = nullptr, const char* const section = ".text");
	void* AOBScanModule(const std::string& aob, const ptrdiff_t offset, const char* const module = nullptr, const char* const section = ".text");
	
	// Extract a base pointer from a RIP-relative instruction
	// by default, 64 bit mov is the target, but this can be overriden with instructionOffset and opcodeOffset
	void* AOBScanBase(const std::string& aob, const ptrdiff_t opcodeOffset = 3, const ptrdiff_t instructionOffset = 7, const char* const module = nullptr, const char* const section = ".text");

	// Hook a function with Minhook
	void Hook(const std::string& aob, void* dest_func, const char* const module = nullptr, const char* const section = ".text");
	void Hook(const std::string& aob, void* dest_func, const ptrdiff_t offset, const char* const module = nullptr, const char* const section = ".text");

	// Patch a function with BytePatch
	void Patch(const std::string& aob, const std::string& replacementBytes, const char* const module = nullptr, const char* const section = ".text");
	void Patch(const std::string& aob, const std::string& replacementBytes, const ptrdiff_t offset, const char* const module = nullptr, const char* const section = ".text");

	// Required: MinHook setup.
	void Initialize();

	// Apply all hooks that have been registered with Hook
	void Apply();
	
	// Remove all registered hooks.
	void Unhook();
}
#endif