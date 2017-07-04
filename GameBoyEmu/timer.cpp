#include "stdafx.h"
#include "timer.h"

void Timer::step_ahead(u32 cycles)
{
	//~4kHz, ~262kHz, ~65,5kHz, ~16kHz
	static const u32 tick_cycles[] = {1024, 16, 64, 256};

	//divider ticks at frequency ~16kHz
	divider_cycles += cycles;
	divider += divider_cycles / 256;
	divider_cycles %= 256;

	if (enabled)
	{
		counter_cycles += cycles;

		while (counter_cycles >= tick_cycles[control])
		{
			if (counter != 0xFF)
				++counter;

			else
			{
				counter = mod;
				interrupts.raise(INT_TIMER);
			}

			counter_cycles -= tick_cycles[control];
		}
	}
}

void Timer::step(u32 cycles)
{
	step_ahead(cycles - cycles_ahead);
	cycles_ahead = 0;
}

u8 Timer::read_byte(u16 adress, u32 cycles_passed)
{
	cycles_passed -= cycles_ahead;

	if (cycles_passed > 0)
	{
		step_ahead(cycles_passed);
		cycles_ahead += cycles_passed;
	}

	if (adress == 0xFF04)
		return divider;

	else if (adress == 0xFF05)
		return counter;

	else if (adress == 0xFF06)
		return mod;

	else if (adress == 0xFF07)
		return change_bit(control, enabled, 2);
}

void Timer::write_byte(u16 adress, u8 value, u32 cycles_passed)
{
	cycles_passed -= cycles_ahead;

	if (cycles_passed > 0)
	{
		step_ahead(cycles_passed);
		cycles_ahead += cycles_passed;
	}

	if (adress == 0xFF04)
	{
		static const u8 fault_bits[] = { 9, 3, 5, 7 };

		if (check_bit(counter_cycles, fault_bits[control]))
			++counter;

		divider = 0;
		divider_cycles = 0;
		counter_cycles = 0;
	}

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
