#include "square_synth.h"

u32 SquareSynth::calculate_freq()
{
	u32 new_freq = sweep_shadow >> sweep_shift;
	new_freq = sweep_shadow + (sweep_neg ? -new_freq : new_freq);

	if (new_freq > 2047)
		enabled = false;

	if (sweep_neg)
		sweep_calculated = true;

	return new_freq;
}

void SquareSynth::start_playing()
{
	enabled = true;
	sweep_calculated = false;

	if (length_counter == 0)
		length_counter = 64;

	timer = (2048 - freq) * 2;
	envelope_enabled = true;
	envelope_counter = start_envelope;
	volume = volume_load;
	sweep_shadow = freq;
	sweep_counter = sweep_load;

	if (sweep_counter == 0)
		sweep_counter = 8;

	sweep_enabled = (sweep_load > 0) || (sweep_shift > 0);

	if (sweep_shift > 0)
		calculate_freq();
}

SquareSynth::SquareSynth()
{
	reset();
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
	sweep_calculated = false;
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

void SquareSynth::step(u32 cycles, u8* sample_buffer)
{
	static const u8 duty_lut[] = { 0x01, 0x81, 0x87, 0x7E };
	u32 pos = 0;

	while (cycles >= timer)
	{
		u32 cycles_spend = timer;

		cycles -= timer;
		timer = (2048 - freq) * 2;
		duty_pos = (duty_pos + 1) % 8;

		out_vol = enabled && dac_enabled && check_bit(duty_lut[duty], duty_pos) ? volume : 0;
		std::memset(&sample_buffer[pos], out_vol, sizeof(u8) * cycles_spend);
		pos += cycles_spend;
	}

	timer -= cycles;
	std::memset(&sample_buffer[pos], out_vol, sizeof(u8) * cycles);
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

void SquareSynth::write_reg(u16 reg_num, u8 value, u32 seq_frame)
{
	if (reg_num == 0)
	{
		const bool old_neg = sweep_neg;

		sweep_load = (value >> 4) & 0x7;
		sweep_neg = check_bit(value, 3);
		sweep_shift = value & 0x7;

		//apu quirk: if we change mode: neg -> pos, after sweep calc, we disable it
		if (old_neg && !sweep_neg && sweep_calculated && enabled)
			enabled = false;
	}

	else if (reg_num == 1)
	{
		duty = (value >> 6) & 0x3;
		length_counter = 64 - (value & 0x3F);
	}

	else if (reg_num == 2)
	{
		const bool old_env_asc = envelope_asc;
		const u32 old_env = start_envelope;

		dac_enabled = (value & 0xF8) != 0;
		volume_load = (value >> 4) & 0xF;
		envelope_asc = check_bit(value, 3);
		start_envelope = value & 0x7;

		envelope_counter = start_envelope;
		volume = volume_load;

		if (!dac_enabled)
			enabled = false;

		//zombie mode (CGB02/04 version)
		if (enabled)
		{
			if (old_env == 0 && envelope_enabled)
				++volume;

			else if (!old_env_asc)
				volume += 2;

			if (old_env_asc ^ envelope_asc) //consistent across CGB
				volume = 16 - volume;

			volume &= 0xF; //consistent across CGB
		}
	}

	else if (reg_num == 3)
		freq = (freq & 0x700) | value;

	else if (reg_num == 4)
	{
		const bool old_enable = length_enabled;
		const bool len_enable = check_bit(value, 6);

		length_enabled = len_enable;
		freq = (freq & 0xFF) | ((value & 0x7) << 8);

		//apu quirk: additional length counter 'ticks'
		if (!old_enable && len_enable && ((seq_frame % 2) == 1))
			update_length();

		if (check_bit(value, 7))
			start_playing();

		//apu quirk: another additional len ctr 'tick'
		if (((seq_frame % 2) == 1) && length_counter == 64 && (value & 0xC0) == 0xC0)
			length_counter = 63;
	}
}

u8 SquareSynth::get_volume() const
{
	return out_vol;
}

void SquareSynth::serialize(std::ostream& stream)
{
	stream << length_counter << timer << duty_pos << duty;
	stream << envelope_counter << start_envelope << freq;
	stream << sweep_counter << sweep_shadow << sweep_shift;
	stream << sweep_load << volume_load << volume << out_vol;

	stream << enabled << length_enabled << envelope_enabled;
	stream << envelope_asc << dac_enabled << sweep_enabled << sweep_neg;
}

void SquareSynth::deserialize(std::istream& stream)
{
	stream >> length_counter >> timer >> duty_pos >> duty;
	stream >> envelope_counter >> start_envelope >> freq;
	stream >> sweep_counter >> sweep_shadow >> sweep_shift;
	stream >> sweep_load >> volume_load >> volume >> out_vol;

	stream >> enabled >> length_enabled >> envelope_enabled;
	stream >> envelope_asc >> dac_enabled >> sweep_enabled >> sweep_neg;
}
