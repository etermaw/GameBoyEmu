#pragma once
#include "stdafx.h"

class NoiseSynth
{
	private:
		u32 length_counter;
		u32 envelope_counter;
		u32 envelope_load;
		u32 timer;
		u32 current_divisor;
		u32 clock_shift;
		u32 volume_load;
		u16 lfsr;
		u8 volume;
		u8 out_vol;

		bool enabled;
		bool envelope_asc;
		bool length_enabled;
		bool envelope_enabled;
		bool width_mode;
		bool dac_enabled;

		void start_playing();

	public:
		NoiseSynth() { reset(); }

		void reset();
		void update_length();		
		void update_envelope();
		void step(u32 cycles);
		
		u8 read_reg(u16 reg_num);
		void write_reg(u16 reg_num, u8 value);
};