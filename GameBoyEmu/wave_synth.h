#pragma once
#include "stdafx.h"

class WaveSynth
{
	private:
		u16 wave_ram[16];
		u32 length_counter;
		u32 sound_length;
		u32 timer;
		u32 buffer_pos;
		u8 freq_low;
		u8 freq_high;
		u8 output_level;
		u8 out_volume;

		bool enabled;
		bool dac_enabled;
		bool length_enabled;

		void start_playing();

	public:
		WaveSynth() { memset(this, 0, sizeof(*this)); }

		void update_length();
		void step(u32 cycles);

		u8 read_reg(u16 reg_num);
		void write_reg(u16 reg_num, u8 value);

		u8 read_ram(u16 adress);
		void write_ram(u16 adress, u8 value);
};