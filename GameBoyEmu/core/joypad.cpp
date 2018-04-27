////#include "stdafx.h"
#include "joypad.h"
#include "utils/bit_ops.h"

u8 Joypad::read_byte(u16 adress, u32 cycles_passed)
{
	UNUSED(adress);
	UNUSED(cycles_passed);
	
	u8 ret = 0xFF;
	
	if (dir_keys)
		ret = combine_bits(!sel_keys, !dir_keys, keys[3], keys[2], keys[1], keys[0]);

	else if (sel_keys)
		ret = combine_bits(!sel_keys, !dir_keys, keys[7], keys[6], keys[5], keys[4]);

	return ret; 
}

void Joypad::write_byte(u16 adress, u8 value, u32 cycles_passed)
{
	UNUSED(adress);
	UNUSED(cycles_passed);
	
	dir_keys = !check_bit(value, 4); //if bit 4 is zero, then we select direction keys
	sel_keys = !check_bit(value, 5); //if bit 5 is zero, then we select special keys
}

void Joypad::push_key(KEYS keycode)
{
	keys[keycode] = false;
}

void Joypad::release_key(KEYS keycode)
{
	keys[keycode] = true;
}

void Joypad::serialize(std::ostream& stream)
{
	stream.write(reinterpret_cast<char*>(keys), sizeof(bool) * 8);
	stream << dir_keys << sel_keys;
}

void Joypad::deserialize(std::istream& stream)
{
	stream.read(reinterpret_cast<char*>(keys), sizeof(bool) * 8);
	stream >> dir_keys >> sel_keys;
}
