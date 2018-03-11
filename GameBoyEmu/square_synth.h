#pragma once
#include "stdafx.h"

class SquareSynth
{
	private:
		u32 length_counter = 0;
		u32 timer = 0;
		u32 duty_pos = 0;
		u32 duty = 0;
		u32 envelope_counter = 0;
		u32 start_envelope = 0;
		u32 freq = 0;
		u32 sweep_counter = 0;
		u32 sweep_shadow = 0;
		u32 sweep_shift = 0;
		u32 sweep_load = 0;
		u8 volume_load = 0;
		u8 volume = 0;
		u8 out_vol = 0;

		bool enabled = false;
		bool length_enabled = false;
		bool envelope_enabled = false;
		bool envelope_asc = false;
		bool dac_enabled = false;
		bool sweep_enabled = false;
		bool sweep_neg = false;
		bool sweep_calculated = false;

		u32 calculate_freq();
		void start_playing();

	public:
		SquareSynth();

		bool is_enabled() const;
		void reset();
		void update_sweep();
		void update_length();
		void update_envelope();
		void step(u32 cycles, u8* sample_buffer = nullptr);

		u8 read_reg(u16 reg_num);
		void write_reg(u16 reg_num, u8 value, u32 seq_frame);
		
		u8 get_volume() const;

		void serialize(std::ostream& stream);
		void deserialize(std::istream& stream);
};