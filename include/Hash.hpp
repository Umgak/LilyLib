/*
	* LilyLib::Hash
	* Implementation of the FNV-1a algorithm for faster lookups in ::Memory
	* You probably don't need to #include this

	* Copyright (c) 2026 Lily

	* This program is free software; licensed under the MIT license.
	* You should have received a copy of the license along with this program.
	* If not, see <https://opensource.org/licenses/MIT>.
*/
#include <stdint.h>

namespace LilyLib::Hash {
	// constants (from wikipedia lol)
	inline constexpr uint64_t basis = 0xcbf29ce484222325;
	inline constexpr uint64_t prime = 0x100000001b3;
	inline constexpr uint64_t hash_64(const char* const str, const uint64_t hash = basis) noexcept {
		return (str[0] == '\0') ? hash : hash_64(&str[1], (hash ^ uint64_t((uint8_t)str[0]))*prime);
	}
}