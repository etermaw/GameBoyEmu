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
		u8 volume;
		u8 out_vol;

		bool enabled;
		bool length_enabled;
		bool envelope_enabled;
		bool envelope_asc;
		bool dac_enabled;

	public:
		void update_sweep();
		void update_length();
		void update_envelope();
		void step(u32 cycles);
};