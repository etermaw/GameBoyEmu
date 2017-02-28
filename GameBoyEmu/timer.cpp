#include "stdafx.h"
#include "timer.h"

bool Timer::step(u32 cycles)
{
	//~4kHz, ~262kHz, ~65,5kHz, ~16kHz
	static const u32 tick_cycles[] = {1024, 16, 64, 256};

	bool raise_interrupt = false;

	divider_cycles += cycles;

	//divider ticks at frequency ~16kHz
	while (divider_cycles >= 256)
	{
		divider++;
		divider_cycles -= 256;
	}

	if (enabled)
	{
		counter_cycles += cycles;

		while (counter_cycles >= tick_cycles[control])
		{
			if (counter == 0xFF)
			{
				counter = mod;
				raise_interrupt = true;
			}

			else
				counter++;

			counter_cycles -= tick_cycles[control];
		}
	}

	return raise_interrupt;
}

u8 Timer::read_byte(u16 adress)
{
	if (adress == 0xFF04)
		return divider;

	else if (adress == 0xFF05)
		return counter;

	else if (adress == 0xFF06)
		return mod;

	else if (adress == 0xFF07)
		return change_bit(control, enabled, 2);
}

void Timer::write_byte(u16 adress, u8 value)
{
	if (adress == 0xFF04)
		divider = 0;

	else if (adress == 0xFF05)
		counter = value;

	else if (adress == 0xFF06)
		mod = value;

	else if (adress == 0xFF07)
	{
		control = value & 0x3;
		enabled = check_bit(value, 2);
	}
}
