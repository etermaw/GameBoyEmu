#include "wave_synth.h"

void WaveSynth::start_playing()
{
	timer = (2048 - ((freq_high << 8) | freq_low)) * 2;
	buffer_pos = 0;

	if (length_counter == 0)
		length_counter = 256;

	enabled = true;
}

bool WaveSynth::is_enabled() const
{
	return enabled && dac_enabled;
}

void WaveSynth::reset()
{
	length_counter = 0;
	timer = 0;
	buffer_pos = 0;
	freq_low = 0;
	freq_high = 0;
	output_level = 0;
	out_volume = 0;

	enabled = false;
	dac_enabled = false;
	length_enabled = false;
}

void WaveSynth::update_length()
{
	if (length_enabled)
	{
		--length_counter;

		if (length_counter == 0)
			enabled = false;
	}
}

void WaveSynth::step(u32 cycles)
{
	while (cycles > timer)
	{
		cycles -= timer;
		timer = (2048 - ((freq_high << 8) | freq_low)) * 2;
		buffer_pos = (buffer_pos + 1) % 32;

		if (enabled && dac_enabled)
		{
			u8 sample = wave_ram[buffer_pos / 2];
			sample = (buffer_pos % 2 == 0 ? sample : (sample >> 4)) & 0x0F;

			out_volume = output_level != 0 ? (sample >> (output_level - 1)) : 0;
		}

		else
			out_volume = 0;
	}

	timer -= cycles;
}

u8 WaveSynth::read_reg(u16 reg_num)
{
	if (reg_num == 0)
		return change_bit<u8>(0xFF, dac_enabled, 7);

	else if (reg_num == 2)
		return (output_level << 5) | 0x9F;

	else if (reg_num == 4)
		return change_bit<u8>(0xFF, length_enabled, 6); //freq is write only

	else 
		return 0xFF; //regs 1,3 are write only, so they return 0xFF
}

void WaveSynth::write_reg(u16 reg_num, u8 value)
{
	if (reg_num == 0)
	{
		dac_enabled = check_bit(value, 7);

		if (!dac_enabled)
			enabled = false;
	}

	else if (reg_num == 1)
		length_counter = 256 - value;

	else if (reg_num == 2)
		output_level = (value >> 5) & 0x3;

	else if (reg_num == 3)
		freq_low = value;

	else if (reg_num == 4)
	{
		length_enabled = check_bit(value, 6);
		freq_high = value & 0x7;

		if (check_bit(value, 7))
			start_playing();
	}
}

u8 WaveSynth::read_ram(u16 adress)
{
	return wave_ram[adress];
}

void WaveSynth::write_ram(u16 adress, u8 value)
{
	wave_ram[adress] = value;
}
