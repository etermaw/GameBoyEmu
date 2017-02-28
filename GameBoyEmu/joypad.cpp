#include "stdafx.h"
#include "joypad.h"
#include "bit_ops.h"

u8 Joypad::read_byte(u16 adress)
{
	u8 ret = 0xFF;
	
	if (dir_keys)
		ret = combine_bits(keys[0], keys[1], keys[2], keys[3], !dir_keys, dir_keys);

	else
		ret = combine_bits(keys[4], keys[5], keys[6], keys[7], !dir_keys, dir_keys);

	return ret; 
}

void Joypad::write_byte(u16 adress, u8 value)
{
	dir_keys = ((value & 0x30) == 0x20);//if bit 4 is zero, then we select direction keys
}

void Joypad::push_key(KEYS keycode)
{
	keys[keycode] = false;
}

void Joypad::release_key(KEYS keycode)
{
	keys[keycode] = true;
}
