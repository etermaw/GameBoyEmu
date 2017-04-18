#pragma once
template<class T>
constexpr inline T set_bit(T num, size_t pos)
{
	return num | (1 << pos);
}

template<class T>
constexpr inline T clear_bit(T num, size_t pos)
{
	return num & ~(1 << pos);
}

template<class T>
constexpr inline T toggle_bit(T num, size_t pos)
{
	return num ^ (1 << pos);
}

template<class T>
constexpr inline T change_bit(T num, bool value, size_t pos)
{
	return num ^ ((-value ^ num) & (1 << pos));
}

template<class T>
constexpr inline bool check_bit(T num, size_t pos)
{
	return (num >> pos) & 1;
}

//swap all bits eg. flip_bits(00110010) == 01001100
inline u8 flip_bits(u8 b)
{
	b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
	b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
	b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
	return b;
}

//combine up to 8 bits (pad with 0) into 1 byte
//eg. combine_bits(1,1,0,0,1,0,0,1) == 11001001, combine_bits(1,1) == 00000011
constexpr inline u8 combine_bits(bool first, bool lowest)
{
	return (static_cast<u8>(first) << 1) | static_cast<u8>(lowest);
}

template<class... T>
constexpr inline u8 combine_bits(bool highest, T... args)
{
	static_assert(sizeof...(args) < 8, "Combine_bits takes max 8 bits!");

	return (static_cast<u8>(highest) << (sizeof...(args))) | combine_bits(args...);
}

template<class U>
inline U ror(U num, u32 shift)
{
	static_assert(std::is_integral<U>::value && std::is_unsigned<U>::value, "Ror is only for unsigned ints!");
	
	return (num >> shift) | (num << (sizeof(U) * CHAR_BIT - shift));
}

template<class U>
inline U rol(U num, u32 shift)
{
	static_assert(std::is_integral<U>::value && std::is_unsigned<U>::value, "Rol is only for unsigned ints!");
	
	return (num << shift) | (num >> (sizeof(U) * CHAR_BIT - shift));
}