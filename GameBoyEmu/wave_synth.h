#pragma once
#include "stdafx.h"

class WaveSynth
{
	private:
		u32 length_counter = 0;
		u32 timer = 0;
		u32 buffer_pos = 0;
		u16 wave_ram[16];
		u8 freq_low = 0;
		u8 freq_high = 0;
		u8 output_level = 0;
		u8 out_volume = 0;

		bool enabled = false;
		bool dac_enabled = false;
		bool length_enabled = false;

		void start_playing();

	public:
		WaveSynth();

		bool is_enabled() const;
		void reset();

		void update_length();
		void step(u32 cycles, u8* sample_buffer = nullptr);

		u8 read_reg(u16 reg_num);
		void write_reg(u16 reg_num, u8 value, u32 seq_frame);

		u8 read_ram(u16 adress);
		void write_ram(u16 adress, u8 value);

		u8 get_volume() const;

		void serialize(std::ostream& stream);
		void deserialize(std::istream& stream);
};