/*
	* LilyLib::Memory
	* Functions for scanning memory
	* Requires Pattern16

	* Copyright (c) 2026 Lily

	* This program is free software; licensed under the MIT license.
	* You should have received a copy of the license along with this program.
	* If not, see <https://opensource.org/licenses/MIT>.
*/
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif
#include <Psapi.h>
#include "../include/Memory.hpp"
#include "../include/Exception.hpp"

#include "../deps/Pattern16/include/Pattern16.h"
#include "../deps/minhook/include/MinHook.h"
#include "../deps/BytePatch/include/BytePatch.hpp"

namespace LilyLib::Memory {
	struct MemoryRegion {
		void *start = 0;
		size_t size = 0;

		MemoryRegion(const char* module, const char* pe_section) {
			HMODULE hModule = GetModuleHandleA(module);
			MODULEINFO modInfo{};
			
			if (!GetModuleInformation(GetCurrentProcess(), hModule, &modInfo, sizeof(modInfo)))
			{
				throw DetailedException(std::source_location::current(), 
					"Failed to create MemoryRegion: GetModuleInformation returned {} while looking for section {} of module {}.", 
					GetLastError(), 
					pe_section, 
					module);
			}
			uintptr_t base = (uintptr_t)modInfo.lpBaseOfDll;

			IMAGE_DOS_HEADER* dosHeader = (IMAGE_DOS_HEADER*)base;
			IMAGE_NT_HEADERS* ntHeader = (IMAGE_NT_HEADERS*)((uintptr_t)base + dosHeader->e_lfanew);
			IMAGE_SECTION_HEADER* section = IMAGE_FIRST_SECTION(ntHeader);

			for (int i = 0; i < ntHeader->FileHeader.NumberOfSections; i++) {
				if (memcmp(pe_section, (char*)section[i].Name, IMAGE_SIZEOF_SHORT_NAME) == 0) {
					start = (void*)(base + section[i].VirtualAddress);
					size = (size_t)section[i].Misc.VirtualSize;
				}
			}
		}
	};

	void* AOBScanModule(const std::string& aob, const char* module, const char* section)
	{
		void* address = nullptr;
		try {
			MemoryRegion mem(module, section);
			address = Pattern16::scan(mem.start, mem.size, aob);                
		} catch (...) {
			std::throw_with_nested(DetailedException(std::source_location::current(),
				"While attempting to locate pattern: {}", aob));
		}
		if (address == nullptr)
		{
			throw DetailedException(std::source_location::current(),
				"Failed to locate pattern: {}", aob);
		}
		return address;
	}

	void* AOBScanModule(const std::string& aob, size_t offset, const char* module, const char* section)
	{
		char* address = (char*)AOBScanModule(aob, module, section);
		return reinterpret_cast<void*>(address + offset);
	}

	void* AOBScanBase(const std::string& aob, const size_t opcodeOffset, const size_t instructionOffset, const char* module, const char* section)
	{
		try {
			char* address = (char*)AOBScanModule(aob, module, section);
			return reinterpret_cast<void*>(address + *reinterpret_cast<int32_t*>(address + opcodeOffset) + instructionOffset);
		} catch (...) {
			std::throw_with_nested(DetailedException(std::source_location::current(),
				"Failed to locate base."));
		}
	}

	void Hook(const std::string& aob, void* dest_func, const char* module, const char* section)
	{
		Hook(aob, dest_func, 0, module, section);
	}

	void Hook(const std::string& aob, void* dest_func, size_t offset, const char* module, const char* section)
	{
		void* source_func;
		try {
			source_func = AOBScanModule(aob, offset, module, section);
		} catch (...) {
			std::throw_with_nested(DetailedException(std::source_location::current(),
				"Failed to locate function to hook."));
		}
		if (MH_CreateHook(source_func, dest_func, nullptr) != MH_OK)
		{
			throw LilyLib::DetailedException(std::source_location::current(),
				"Failed to hook function.");
		}
		MH_QueueEnableHook(source_func);
	}

	void Patch(const std::string& aob, const std::string& replacementBytes, const char* module, const char* section) 
	{
		Patch(aob, replacementBytes, 0, module, section);
	}

	void Patch(const std::string& aob, const std::string& replacementBytes, size_t offset, const char* module, const char* section)
	{
		void* source_pointer;
		try {
			source_pointer = AOBScanModule(aob, offset, module, section);
		} catch (...) {
			std::throw_with_nested(DetailedException(std::source_location::current(),
				"Failed to locate destination to write BytePatch."));
		}
		if (BP_CreatePatch(source_pointer, replacementBytes) != BP_OK)
		{
			throw LilyLib::DetailedException(std::source_location::current(),
				"Failed to create BytePatch.");
		}
		BP_QueueEnablePatch(source_pointer);
	}

	void Initialize()
	{
		if (MH_Initialize() != MH_OK)
		{
			throw LilyLib::DetailedException(std::source_location::current(), "Failed to initialize MinHook.");
		}
	}

	void Apply()
	{
		if (MH_ApplyQueued() != MH_OK)
		{
			throw LilyLib::DetailedException(std::source_location::current(), "MinHook failed to apply queued hooks.");
		}
		if (BP_ApplyQueued() != BP_OK)
		{
			throw LilyLib::DetailedException(std::source_location::current(), "BytePatch failed to apply queued patches.");
		}
	}
	
	void Unhook()
	{
		MH_DisableHook(MH_ALL_HOOKS);
		if (MH_Uninitialize() != MH_OK)
		{
			throw LilyLib::DetailedException(std::source_location::current(), "Failed to uninitialize MinHook.");
		}
		if (BP_DisablePatch(BP_ALL_PATCHES) != BP_OK)
		{
			throw LilyLib::DetailedException(std::source_location::current(), "Failed to restore BytePatches.");
		}
	}
}
