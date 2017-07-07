#pragma once
#include "stdafx.h"
#include "IMemory.h"
#include "square_synth.h"
#include "wave_synth.h"
#include "noise_synth.h"

class APU final : public IMemory
{
	private:
		SquareSynth channel_1, 
					channel_2;
		WaveSynth channel_3;
		NoiseSynth channel_4;

		u32 cycles_ahead = 0;
		u32 sequencer_cycles = 0;
		u32 sequencer_frame = 0;
		u8 dummy_regs[2] = {};

		bool enabled = false;
		bool double_speed = false;

		void step_ahead(u32 cycles);

	public:
		APU();
		
		u8 read_byte(u16 adress, u32 cycles_passed) override;
		void write_byte(u16 adress, u8 value, u32 cycles_passed) override;

		void step(u32 cycles);
		void set_speed(bool speed) { double_speed = speed; }
};
