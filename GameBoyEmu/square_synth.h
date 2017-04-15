#pragma once
#include "stdafx.h"

class SquareSynth
{
	private:
		u32 length_counter;
		u32 timer;
		u32 duty_pos;
		u32 duty;
		u32 envelope_counter;
		u32 start_envelope;
		u32 freq;
		u32 sweep_counter;
		u32 sweep_shadow;
		u32 sweep_shift;
		u32 sweep_load;
		u8 volume_load;
		u8 volume;
		u8 out_vol;

		bool enabled;
		bool length_enabled;
		bool envelope_enabled;
		bool envelope_asc;
		bool dac_enabled;
		bool sweep_enabled;
		bool sweep_neg;

		u32 calculate_freq();
		void start_playing();

	public:
		SquareSynth() { reset(); }

		void reset();
		void update_sweep();
		void update_length();
		void update_envelope();
		void step(u32 cycles);

		u8 read_reg(u16 reg_num);
		void write_reg(u16 reg_num, u8 value);
};