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
#include <windows.h>
#endif
#include <psapi.h>
#include <unordered_map>
#include <utility>
#include <vector>
#include "../include/Memory.hpp"
#include "../include/Exception.hpp"

#include "../deps/Pattern16/include/Pattern16.h"
#include "../deps/minhook/include/MinHook.h"
#include "../deps/BytePatch/include/BytePatch.hpp"

namespace LilyLib::Memory {
	struct MemRegionCache {
		struct MemoryRegion {
			void* start = 0;
			size_t size = 0;
		};
		// cache for memory regions
		static inline std::unordered_map<std::string,
			std::unordered_map<std::string, std::vector<MemoryRegion>>> _data;

		static void cacheModule(const char* module) {
			std::string moduleName = (module == nullptr) ? "MAIN_EXE" : module;
			if (_data.contains(moduleName)) return;
			HMODULE hModule = GetModuleHandleA(module);
			if (!hModule) {
				throw DetailedException(std::source_location::current(),
					"GetModuleHandleA failed for module: {}.\nError: {}", moduleName, GetLastError());
			}

			MODULEINFO modInfo{};
			if (!GetModuleInformation(GetCurrentProcess(), hModule, &modInfo, sizeof(modInfo)))
			{
				throw DetailedException(std::source_location::current(),
					"Failed to create MemoryRegion map!\nGetModuleInformation returned {} while looking for module {}.",
					GetLastError(),
					moduleName);
			}
			uintptr_t base = reinterpret_cast<uintptr_t>(modInfo.lpBaseOfDll);
			auto* dosHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(base);
			auto* ntHeader = reinterpret_cast<IMAGE_NT_HEADERS*>(base + dosHeader->e_lfanew);
			auto* section = IMAGE_FIRST_SECTION(ntHeader);

			for (int i = 0; i < ntHeader->FileHeader.NumberOfSections; i++) {
				char nameBuf[IMAGE_SIZEOF_SHORT_NAME + 1]{};
				memcpy(nameBuf, section[i].Name, IMAGE_SIZEOF_SHORT_NAME);
				std::string sectionName = nameBuf;

				auto start = reinterpret_cast<void*>(base + section[i].VirtualAddress);
				auto size = static_cast<size_t>(section[i].Misc.VirtualSize);
				_data[moduleName][sectionName].emplace_back(start, size);
			}
		}

		static const std::vector<MemoryRegion>& Get(const char* module, const char* section) {
			std::string moduleName = (module == nullptr) ? "MAIN_EXE" : module;
			try {
				cacheModule(module);
			}
			catch (...) {
				std::throw_with_nested(DetailedException(std::source_location::current(),
					"While attempting to cache module {}", moduleName));
			}
			auto moduleIt = _data.find(moduleName);
			if (moduleIt == _data.end()) {
				throw DetailedException(std::source_location::current(),
					"Failed to locate module {} in map!", moduleName);
			}
			auto sectionIt = moduleIt->second.find(section);
			if (sectionIt == moduleIt->second.end()) {
				throw DetailedException(std::source_location::current(),
					"Failed to locate section {} of module {}!", section, moduleName);
			}
			return sectionIt->second;
		}
	};

	void* AOBScanModule(const std::string& aob, const char* module, const char* section)
	{
		try {
			const auto& regions = MemRegionCache::Get(module, section);
			for (const auto& region : regions)
			{
				void* address = Pattern16::scan(region.start, region.size, aob);
				if (address) return address;
			}
		} catch (...) {
			std::throw_with_nested(DetailedException(std::source_location::current(),
				"While attempting to locate pattern: {}", aob));
		}
		// failed, panic
		throw DetailedException(std::source_location::current(),
			"Failed to locate pattern: {}", aob);
	}

	void* AOBScanModule(const std::string& aob, const ptrdiff_t offset, const char* module, const char* section)
	{
		void* address = AOBScanModule(aob, module, section);
		return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(address) + offset);
	}

	void* AOBScanBase(const std::string& aob, const ptrdiff_t opcodeOffset, const ptrdiff_t instructionOffset, const char* module, const char* section)
	{
		try {
			uintptr_t address = reinterpret_cast<uintptr_t>(AOBScanModule(aob, module, section));
			return reinterpret_cast<void*>(address + instructionOffset + *reinterpret_cast<int32_t*>(address+opcodeOffset));
		} catch (...) {
			std::throw_with_nested(DetailedException(std::source_location::current(),
				"Failed to locate base."));
		}
	}

	void Hook(const std::string& aob, void* dest_func, const char* module, const char* section)
	{
		Hook(aob, dest_func, 0, module, section);
	}

	void Hook(const std::string& aob, void* dest_func, const ptrdiff_t offset, const char* module, const char* section)
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

	void Patch(const std::string& aob, const std::string& replacementBytes, const ptrdiff_t offset, const char* module, const char* section)
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
