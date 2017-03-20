#include "noise_synth.h"

static const u32 divisor[] = { 8,16,32,48,64,80,96,112 };

void NoiseSynth::start_playing()
{
	lfsr = 0x7FFF;
	timer = divisor[current_divisor] << clock_shift;
	volume = volume_load;
	envelope_counter = envelope_load;

	if (length_counter == 0)
		length_counter = 64;

	envelope_enabled = true;
	enabled = true;
}

void NoiseSynth::update_length()
{
	if (length_enabled && length_counter > 0)
	{
		--length_counter;

		if (length_counter == 0)
			enabled = false;
	}
}

void NoiseSynth::update_envelope()
{
	--envelope_counter;

	if (envelope_counter == 0)
	{
		envelope_counter = envelope_load;

		if (envelope_counter == 0) //obscure behaviour
			envelope_counter = 8;

		if (envelope_enabled && envelope_load > 0)
		{
			if (envelope_asc && volume < 15)
				++volume;

			else if (!envelope_asc && volume > 0)
				--volume;
		}

		if (volume == 0 || volume == 15)
			envelope_enabled = false;
	}
}

void NoiseSynth::step(u32 cycles)
{
	const u32 timer_value = divisor[current_divisor] << clock_shift;

	//TODO: what if in the middle something changes (length, envelope)?
	while (cycles > timer)
	{
		cycles -= timer;
		timer = timer_value;

		auto tmp = check_bit(lfsr, 1) ^ check_bit(lfsr, 0);
		lfsr = change_bit(lfsr >> 1, tmp, 15);

		if (width_mode)
			lfsr = change_bit(lfsr, tmp, 6);

		out_vol = enabled && dac_enabled && !check_bit(lfsr, 0) ? volume : 0;
	}

	timer -= cycles;
}

u8 NoiseSynth::read_reg(u16 reg_num)
{
	if (reg_num == 0)
		return length_load & 0x3F;

	else if (reg_num == 1)
		return ((volume_load & 0xF) << 4) | (envelope_asc << 3) | (envelope_load & 0x7);

	else if (reg_num == 2)
		return ((clock_shift & 0xF) << 4) | (width_mode << 3) | (current_divisor & 0x7);

	else if (reg_num == 3)
		return length_enabled << 6;

	else
		return 0xFF;
}

void NoiseSynth::write_reg(u16 reg_num, u8 value)
{
	if (reg_num == 0)
	{
		length_load = (value & 0x3F);
		length_counter = 64 - length_load;
	}

	else if (reg_num == 1)
	{
		dac_enabled = (value & 0xF8) != 0;
		volume_load = (value & 0xF0) >> 4;
		envelope_asc = check_bit(value, 3);
		envelope_load = value & 0x7;

		envelope_counter = envelope_load;
		volume = volume_load;
	}

	else if (reg_num == 2)
	{
		clock_shift = (value & 0xF0) >> 4;
		width_mode = check_bit(value, 3);
		current_divisor = value & 0x7;
	}

	else if (reg_num == 3)
	{
		length_enabled = check_bit(value, 6);

		if (check_bit(value, 7))
			start_playing();
	}
}
