#include "stdafx.h"
#include "joypad.h"
#include "bit_ops.h"

u8 Joypad::read_byte(u16 adress, u32 cycles_passed)
{
	u8 ret = 0xFF;
	
	if (dir_keys)
		ret = combine_bits(!sel_keys, !dir_keys, keys[3], keys[2], keys[1], keys[0]);

	else if (sel_keys)
		ret = combine_bits(!sel_keys, !dir_keys, keys[7], keys[6], keys[5], keys[4]);

	return ret; 
}

void Joypad::write_byte(u16 adress, u8 value, u32 cycles_passed)
{
	dir_keys = ((value & 0x30) == 0x20);//if bit 4 is zero, then we select direction keys
	sel_keys = ((value & 0x30) == 0x10);//if bit 5 is zero, then we select special keys
}

void Joypad::push_key(KEYS keycode)
{
	keys[keycode] = false;
}

void Joypad::release_key(KEYS keycode)
{
	keys[keycode] = true;
}
