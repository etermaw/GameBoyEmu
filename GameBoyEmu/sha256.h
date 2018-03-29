#pragma once
#include <array>

using u64 = std::uint64_t;
using u32 = std::uint32_t;
using u8 = std::uint8_t;

std::array<u32, 8> sha256(const u8* data, u64 len);

