#include "square_synth.h"

u32 SquareSynth::calculate_freq()
{
	u32 new_freq = sweep_shadow >> sweep_shift;
	new_freq = sweep_shadow + (sweep_neg ? -new_freq : new_freq);

	if (new_freq > 2047)
		enabled = false;

	return new_freq;
}

void SquareSynth::start_playing()
{
	enabled = true;

	if (length_counter == 0)
		length_counter = 64;

	timer = (2048 - freq) * 4;
	envelope_enabled = true;
	envelope_counter = start_envelope;
	volume = volume_load;
	sweep_shadow = freq;
	sweep_counter = sweep_load;

	if (sweep_counter == 0)
		sweep_counter = 8;

	sweep_enabled = (sweep_counter > 0) || (sweep_shift > 0);

	if (sweep_shift > 0)
		calculate_freq();
}

bool SquareSynth::is_enabled() const
{
	return enabled && dac_enabled;
}

void SquareSynth::reset()
{
	length_counter = 0;
	timer = 0;
	duty_pos = 0;
	duty = 0;
	envelope_counter = 0;
	start_envelope = 0;
	freq = 0;
	sweep_counter = 0;
	sweep_shadow = 0;
	sweep_shift = 0;
	sweep_load = 0;
	volume_load = 0;
	volume = 0;
	out_vol = 0;

	enabled = false;
	length_enabled = false;
	envelope_enabled = false;
	envelope_asc = false;
	dac_enabled = false;
	sweep_enabled = false;
	sweep_neg = false;
}

void SquareSynth::update_sweep()
{
	if (sweep_enabled && --sweep_counter == 0)
	{
		sweep_counter = sweep_load;

		if (sweep_counter == 0)
			sweep_counter = 8;

		if (sweep_load > 0)
		{
			auto new_freq = calculate_freq();

			if (new_freq < 2048 && sweep_shift > 0)
			{
				sweep_shadow = new_freq;
				freq = new_freq;
				calculate_freq();
			}

			calculate_freq(); //is it required?
		}
	}
}

void SquareSynth::update_length()
{
	if (length_enabled && length_counter > 0)
	{
		--length_counter;

		if (length_counter == 0)
			enabled = false;
	}
}

void SquareSynth::update_envelope()
{
	if (envelope_enabled && --envelope_counter == 0)
	{
		envelope_counter = start_envelope;

		if (envelope_counter == 0) //obscure behaviour
			envelope_counter = 8;

		if (envelope_asc && volume < 15)
			++volume;

		else if (!envelope_asc && volume > 0)
			--volume;

		if (volume == 0 || volume == 15)
			envelope_enabled = false;
	}
}

void SquareSynth::step(u32 cycles)
{
	static const u8 duty_lut[] = { 0x01, 0x81, 0x87, 0x7E };

	while (cycles > timer)
	{
		cycles -= timer;

		timer = (2048 - freq) * 4;
		duty_pos = (duty_pos + 1) % 8;

		out_vol = enabled && dac_enabled && check_bit(duty_lut[duty], duty_pos) ? volume : 0;
	}

	timer -= cycles;
}

u8 SquareSynth::read_reg(u16 reg_num)
{
	if (reg_num == 0)
		return 0x80 | (sweep_load << 4) | (sweep_neg << 3) | (sweep_shift);

	else if (reg_num == 1)
		return (duty << 6) | 0x3F;

	else if (reg_num == 2)
		return (volume_load << 4) | (envelope_asc << 3) | (envelope_counter);

	else if (reg_num == 4)
		return (length_enabled << 6) | 0xBF;

	else
		return 0xFF; //reg 3 is write-only!
}

void SquareSynth::write_reg(u16 reg_num, u8 value)
{
	if (reg_num == 0)
	{
		sweep_load = (value >> 4) & 0x7;
		sweep_neg = check_bit(value, 3);
		sweep_shift = value & 0x7;
	}

	else if (reg_num == 1)
	{
		duty = (value >> 6) & 0x3;
		length_counter = 64 - (value & 0x3F);
	}

	else if (reg_num == 2)
	{
		dac_enabled = (value & 0xF8) != 0;
		volume_load = (value >> 4) & 0xF;
		envelope_asc = check_bit(value, 3);
		start_envelope = value & 0x7;

		envelope_counter = start_envelope;
		volume = volume_load;

		if (!dac_enabled)
			enabled = false;
	}

	else if (reg_num == 3)
		freq = (freq & 0x700) | value;

	else if (reg_num == 4)
	{
		length_enabled = check_bit(value, 6);
		freq = (freq & 0xFF) | ((value & 0x7) << 8);

		if (check_bit(value, 7))
			start_playing();
	}
}
