#include "interrupts.h"

void Interrupts::raise(INTERRUPTS code)
{
	interrupt_flags = set_bit(interrupt_flags, code);
}

bool Interrupts::is_any_raised()
{
	auto masked = interrupt_flags & interrupt_mask;
	return (masked != 0);
}

INTERRUPTS Interrupts::get_first_raised()
{
	u32 counter = 0;
	auto masked_interrupts = interrupt_flags & interrupt_mask;

	while (!check_bit(masked_interrupts, counter))
		counter++;

	interrupt_flags = clear_bit(interrupt_flags, counter);

	return static_cast<INTERRUPTS>(counter);
}

u8 Interrupts::read_byte(u16 adress, u32 cycles_passed)
{
	if (adress == 0xFF0F)
		return interrupt_flags;

	else if (adress == 0xFFFF)
		return interrupt_mask;

	else
		return 0xFF;
}

void Interrupts::write_byte(u16 adress, u8 value, u32 cycles_passed)
{
	if (adress == 0xFF0F)
		interrupt_flags = value & 0x1F;

	else if (adress == 0xFFFF)
		interrupt_mask = value & 0x1F;

	//else ignore
}
