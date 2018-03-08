#include "wave_synth.h"

void WaveSynth::start_playing()
{
	timer = (2048 - ((freq_high << 8) | freq_low));
	buffer_pos = 0;

	if (length_counter == 0)
		length_counter = 256;

	enabled = true;
}

WaveSynth::WaveSynth()
{
	static const u8 init_wave_ram[] = { 0x84,0x40,0x43,0xAA,0x2D,0x78,0x92,0x3C,0x60,0x59,0x59,0xB0,0x34,0xB8,0x2E,0xDA };
	std::memcpy(wave_ram, init_wave_ram, sizeof(u8) * 16);

	reset();
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
	if (length_enabled && length_counter > 0)
	{
		--length_counter;

		if (length_counter == 0)
			enabled = false;
	}
}

void WaveSynth::step(u32 cycles, u8* sample_buffer)
{
	u32 pos = 0;

	while (cycles >= timer)
	{
		u32 cycles_spend = timer;

		cycles -= timer;
		timer = (2048 - ((freq_high << 8) | freq_low));
		buffer_pos = (buffer_pos + 1) % 32;

		if (enabled && dac_enabled && output_level > 0)
		{
			u8 sample = wave_ram[buffer_pos / 2];
			sample = (sample >> (buffer_pos % 2 ? 4 : 0)) & 0x0F;

			out_volume = sample >> (output_level - 1);
		}

		else
			out_volume = 0;

		std::memset(&sample_buffer[pos], out_volume, sizeof(u8) * cycles_spend);
		pos += cycles_spend;
	}

	timer -= cycles;
	std::memset(&sample_buffer[pos], out_volume, sizeof(u8) * cycles);
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

void WaveSynth::write_reg(u16 reg_num, u8 value, u32 seq_frame)
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
		const bool old_enable = length_enabled;
		const bool len_enable = check_bit(value, 6);

		length_enabled = len_enable;
		freq_high = value & 0x7;

		//apu quirk: additional length counter 'ticks'
		if (!old_enable && len_enable && ((seq_frame % 2) == 1))
			update_length();

		if (check_bit(value, 7))
			start_playing();

		//apu quirk: another additional len ctr 'tick'
		if (((seq_frame % 2) == 1) && length_counter == 256 && (value & 0xC0) == 0xC0)
			length_counter = 255;
	}
}

u8 WaveSynth::read_ram(u16 adress)
{
	//apu quirk: if enabled, we get current sample read by channel
	return wave_ram[enabled ? (buffer_pos / 2) : adress];
}

void WaveSynth::write_ram(u16 adress, u8 value)
{
	//apu quirk: if enabled, we set current sample read by channel
	wave_ram[enabled ? (buffer_pos / 2) : adress] = value;
}

void WaveSynth::serialize(std::ostream& stream)
{
	stream.write(reinterpret_cast<char*>(wave_ram), sizeof(u16) * 16);

	stream << length_counter << timer << buffer_pos;
	stream << freq_low << freq_high << output_level;
	stream << out_volume << enabled << dac_enabled << length_enabled;
}

void WaveSynth::deserialize(std::istream & stream)
{
	stream.read(reinterpret_cast<char*>(wave_ram), sizeof(u16) * 16);

	stream >> length_counter >> timer >> buffer_pos;
	stream >> freq_low >> freq_high >> output_level;
	stream >> out_volume >> enabled >> dac_enabled >> length_enabled;
}
