#include "noise_synth.h"

static const u32 divisor[] = { 4,8,16,24,32,40,48,56 };

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

NoiseSynth::NoiseSynth()
{
	reset();
}

bool NoiseSynth::is_enabled() const
{
	return enabled && dac_enabled;
}

void NoiseSynth::reset()
{
	length_counter = 0;
	envelope_counter = 0;
	envelope_load = 0;
	timer = 0;
	current_divisor = 0;
	clock_shift = 0;
	volume_load = 0;
	lfsr = 0;
	volume = 0;
	out_vol = 0;

	enabled = false;
	envelope_asc = false;
	length_enabled = false;
	envelope_enabled = false;
	width_mode = false;
	dac_enabled = false;
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
	if (envelope_enabled && --envelope_counter == 0)
	{
		envelope_counter = envelope_load;

		if (envelope_counter == 0) //obscure behaviour
			envelope_counter = 8;

		if (envelope_load > 0)
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

void NoiseSynth::step(u32 cycles, u8* sample_buffer)
{
	const u32 timer_value = divisor[current_divisor] << clock_shift;
	u32 pos = 0;

	while (cycles >= timer)
	{
		u32 cycles_spend = timer;

		cycles -= timer;
		timer = timer_value;

		auto tmp = check_bit(lfsr, 1) ^ check_bit(lfsr, 0);
		lfsr = change_bit(lfsr >> 1, tmp, 14);

		if (width_mode)
			lfsr = change_bit(lfsr, tmp, 6);

		out_vol = enabled && dac_enabled && !check_bit(lfsr, 0) ? volume : 0;
		std::memset(&sample_buffer[pos], out_vol, sizeof(u8) * cycles_spend);
		pos += cycles_spend;
	}

	timer -= cycles;
	std::memset(&sample_buffer[pos], out_vol, sizeof(u8) * cycles);
}

u8 NoiseSynth::read_reg(u16 reg_num)
{
	if (reg_num == 1)
		return ((volume_load & 0xF) << 4) | (envelope_asc << 3) | (envelope_load & 0x7);

	else if (reg_num == 2)
		return ((clock_shift & 0xF) << 4) | (width_mode << 3) | (current_divisor & 0x7);

	else if (reg_num == 3)
		return change_bit(0xFF, length_enabled, 6);

	else
		return 0xFF; //reg 0 is write only
}

void NoiseSynth::write_reg(u16 reg_num, u8 value, u32 seq_frame)
{
	if (reg_num == 0)
		length_counter = 64 - (value & 0x3F);

	else if (reg_num == 1)
	{
		dac_enabled = (value & 0xF8) != 0;
		volume_load = (value & 0xF0) >> 4;
		envelope_asc = check_bit(value, 3);
		envelope_load = value & 0x7;

		envelope_counter = envelope_load;
		volume = volume_load;

		if (!dac_enabled)
			enabled = false;
	}

	else if (reg_num == 2)
	{
		clock_shift = (value & 0xF0) >> 4;
		width_mode = check_bit(value, 3);
		current_divisor = value & 0x7;
	}

	else if (reg_num == 3)
	{
		const bool old_enable = length_enabled;
		const bool len_enable = check_bit(value, 6);

		length_enabled = len_enable;

		if (!old_enable && len_enable && ((seq_frame % 2) == 1))
			update_length();

		if (check_bit(value, 7))
			start_playing();
	}
}

void NoiseSynth::serialize(std::ostream& stream)
{
	stream << length_counter << envelope_counter << envelope_load;
	stream << timer << current_divisor << clock_shift;
	stream << volume_load << lfsr << volume << out_vol;
	stream << enabled << envelope_asc << length_enabled;
	stream << envelope_enabled << width_mode << dac_enabled;
}

void NoiseSynth::deserialize(std::istream& stream)
{
	stream >> length_counter >> envelope_counter >> envelope_load;
	stream >> timer >> current_divisor >> clock_shift;
	stream >> volume_load >> lfsr >> volume >> out_vol;
	stream >> enabled >> envelope_asc >> length_enabled;
	stream >> envelope_enabled >> width_mode >> dac_enabled;
}
