#include "stdafx.h"
#include "timer.h"

void Timer::step_ahead(u32 cycles)
{
	//~4kHz, ~262kHz, ~65,5kHz, ~16kHz
	static const u32 tick_masks[] = {1023, 15, 63, 255}; //tick cycles {1024, 16, 64, 256}
	static const u32 tick_shifts[] = {10, 4, 6, 8};

	//divider ticks at frequency ~16kHz
	divider_cycles += cycles;
	divider += divider_cycles / 256;
	divider_cycles %= 256;

	reload = false;

	if (overflow && cycles == 4)
		reload = true;

	overflow = false;

	if (enabled)
	{
		counter_cycles += cycles;
		u32 ticks = counter_cycles >> tick_shifts[control];
		counter_cycles &= tick_masks[control];

		if (ticks + counter > 0xFF)
		{
			if (ticks + counter == 0x100)
			{
				if (counter_cycles < 4)
					overflow = true;

				else if (counter_cycles < 8)
					reload = true;
			}

			interrupts.raise(INT_TIMER);
			ticks -= (0x100 - counter);
			counter = mod + ticks % (0x100 - mod);
		}

		else
			counter += ticks;
	}
}

void Timer::check_fault_bits()
{
	static const u8 fault_bits[] = { 9, 3, 5, 7 };

	if (check_bit(counter_cycles, fault_bits[control]))
	{
		if (counter == 0xFF)
		{
			interrupts.raise(INT_TIMER);
			counter = mod; //TODO: oveflow = true; [only when cycles < 4]?
		}

		else
			++counter;
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
		return (overflow) ? 0 : counter;

	else if (adress == 0xFF06)
		return mod;

	else if (adress == 0xFF07)
		return change_bit(control, enabled, 2);

	else
		return 0xFF;
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
		check_fault_bits();

		divider = 0;
		divider_cycles = 0;
		counter_cycles = 0;
	}

	else if (adress == 0xFF05 && !reload)
		counter = value;

	else if (adress == 0xFF06)
	{
		mod = value;

		if (overflow || reload)
			counter = mod;
	}

	else if (adress == 0xFF07)
	{
		if (enabled && !check_bit(value, 2))
			check_fault_bits();

		control = value & 0x3;
		enabled = check_bit(value, 2);
	}
}

void Timer::serialize(std::ostream& stream)
{
	stream << cycles_ahead << divider_cycles << counter_cycles;
	stream << control << divider << counter << mod << enabled;
}

void Timer::deserialize(std::istream& stream)
{
	stream >> cycles_ahead >> divider_cycles >> counter_cycles;
	stream >> control >> divider >> counter >> mod >> enabled;
}
