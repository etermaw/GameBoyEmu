#pragma once
#include "stdafx.h"
#include "IMemory.h"
#include "square_synth.h"
#include "wave_synth.h"
#include "noise_synth.h"

class APU final : public IMemory
{
	private:
		u32 cycles_ahead;
		u32 sequencer_cycles;
		u32 sequencer_frame;

		bool enabled = false;

		//u8 wave_ram[16];
		u8 dummy_regs[22];

		SquareSynth channel_1,channel_2;
		WaveSynth channel_3;
		NoiseSynth channel_4;

		void step_ahead(u32 cycles);

	public:
		APU();
		
		u8 read_byte(u16 adress, u32 cycles_passed) override;
		void write_byte(u16 adress, u8 value, u32 cycles_passed) override;

		void step(u32 cycles);
};