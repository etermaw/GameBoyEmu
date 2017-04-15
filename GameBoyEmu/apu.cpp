#include "apu.h"

APU::APU() : cycles_ahead(0) {}

void APU::step_ahead(u32 cycles)
{
	if (!enabled)
		return;

	sequencer_cycles += cycles;

	if (sequencer_cycles >= 8192)
	{
		sequencer_cycles -= 8192;

		if (sequencer_frame % 2 == 0)
		{
			channel_1.update_length();
			channel_2.update_length();
			channel_3.update_length();
			channel_4.update_length();

			if (sequencer_frame == 2 || sequencer_frame == 6)
				channel_1.update_sweep();
		}

		else if (sequencer_frame == 7)
		{
			channel_1.update_envelope();
			channel_2.update_envelope();
			channel_4.update_envelope();
		}

		sequencer_frame = (sequencer_frame + 1) % 8;
	}

	channel_1.step(cycles);
	channel_2.step(cycles);
	channel_3.step(cycles);
	channel_4.step(cycles);

	//every ~96 cycles take one sample into final buffer (downsampling)
}

u8 APU::read_byte(u16 adress, u32 cycles_passed)
{
	cycles_passed -= cycles_ahead;

	if (cycles_passed > 0)
	{
		step_ahead(cycles_passed);
		cycles_ahead += cycles_passed;
	}

	if (adress >= 0xFF10 && adress <= 0xFF14)
		return channel_1.read_reg(adress - 0xFF10);

	else if (adress >= 0xFF16 && adress <= 0xFF19)
		return channel_2.read_reg(adress - 0xFF15); //why ff15? because we skip 1st register!

	else if (adress >= 0xFF1A && adress <= 0xFF1E)
		return channel_3.read_reg(adress - 0xFF1A);

	else if (adress >= 0xFF20 && adress <= 0xFF23)
		return channel_4.read_reg(adress - 0xFF20);

	else if (adress >= 0xFF24 && adress <= 0xFF25)
		return dummy_regs[adress - 0xFF24];

	else if (adress == 0xFF26)
		return change_bit(0x70, enabled, 7);

	else if (adress >= 0xFF30 && adress <= 0xFF3F)
		return channel_3.read_ram(adress - 0xFF30);

	else
		return 0xFF;
}

void APU::write_byte(u16 adress, u8 value, u32 cycles_passed)
{
	cycles_passed -= cycles_ahead;

	if (cycles_passed > 0)
	{
		step_ahead(cycles_passed);
		cycles_ahead += cycles_passed;
	}

	if (adress >= 0xFF10 && adress <= 0xFF14)
		channel_1.write_reg(adress - 0xFF10, value);

	else if (adress >= 0xFF16 && adress <= 0xFF19)
		channel_2.write_reg(adress - 0xFF15, value); //why ff15? because we skip 1st register!

	else if (adress >= 0xFF1A && adress <= 0xFF1E)
		channel_3.write_reg(adress - 0xFF1A, value);

	else if (adress >= 0xFF20 && adress <= 0xFF23)
		channel_4.write_reg(adress - 0xFF20, value);

	else if (adress >= 0xFF24 && adress <= 0xFF25)
		dummy_regs[adress - 0xFF24] = value;

	else if (adress == 0xFF26)
	{
		enabled = check_bit(value, 7);

		if (!enabled)
		{
			channel_1.reset();
			channel_2.reset();
			channel_3.reset();
			channel_4.reset();

			std::memset(dummy_regs, 0, sizeof(dummy_regs));
		}
	}

	else if (adress >= 0xFF30 && adress <= 0xFF3F)
		channel_3.write_ram(adress - 0xFF30, value);

	//else ignore
}

void APU::step(u32 cycles)
{
	step_ahead(cycles - cycles_ahead);
	cycles_ahead = 0;
}