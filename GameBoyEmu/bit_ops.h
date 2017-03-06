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

inline u8 flip_bits(u8 b)
{
	b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
	b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
	b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
	return b;
}

constexpr inline u8 combine_bits(bool first, bool last)
{
	return (static_cast<u8>(first) << 1) | static_cast<u8>(last);
}

template<class... T>
constexpr inline u8 combine_bits(bool first, T... args)
{
	static_assert(sizeof...(args) < 8, "Combine_bits takes max 8 params!");

	return (static_cast<u8>(first) << (sizeof...(args))) | combine_bits(args...);
}

template<class I>
inline I ror(I num, u32 shift)
{
	static_assert(std::is_integral<I>::value && std::is_unsigned<I>::value, "Ror is only for unsigned ints!");
	//add static_assert to ensure that passed value is 8,16,32,64 bit

	//return (num >> shift) | (num << (sizeof(I) * CHAR_BIT - shift));

	switch (sizeof(I) * CHAR_BIT)
	{
		case 8:
			return _rotr8(num, shift);

		case 16:
			return _rotr16(num, shift);

		case 32:
			return _rotr(num, shift);

		case 64:
			return _rotr64(num, shift);
	}
}

template<class I>
inline I rol(I num, u32 shift)
{
	static_assert(std::is_integral<I>::value && std::is_unsigned<I>::value, "Rol is only for unsigned ints!");
	//add static_assert to ensure that passed value is 8,16,32,64 bit

	//return (num << shift) | ();

	switch (sizeof(I) * CHAR_BIT)
	{
		case 8:
			return _rotl8(num, shift);

		case 16:
			return _rotl16(num, shift);

		case 32:
			return _rotl(num, shift);

		case 64:
			return _rotl64(num, shift);
	}
}