#pragma once
#include "stdafx.h"

class WaveSynth
{
	private:
		u32 length_counter;
		//u32 sound_length;
		u32 timer;
		u32 buffer_pos;
		u16 wave_ram[16];
		u8 freq_low;
		u8 freq_high;
		u8 output_level;
		u8 out_volume;

		bool enabled;
		bool dac_enabled;
		bool length_enabled;

		void start_playing();

	public:
		WaveSynth() 
		{
			static const u8 init_wave_ram[] = { 0x84,0x40,0x43,0xAA,0x2D,0x78,0x92,0x3C,0x60,0x59,0x59,0xB0,0x34,0xB8,0x2E,0xDA };
			std::memcpy(wave_ram, init_wave_ram, sizeof(u8) * 16);
			reset();
		}

		bool is_enabled() const;
		void reset();

		void update_length();
		void step(u32 cycles);

		u8 read_reg(u16 reg_num);
		void write_reg(u16 reg_num, u8 value);

		u8 read_ram(u16 adress);
		void write_ram(u16 adress, u8 value);
};