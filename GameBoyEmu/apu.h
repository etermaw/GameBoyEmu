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

		function<u8**(u8**, u32)> swap_buffers_callback;
		function<void(bool)> enable_audio_callback;
		u8** sample_buffers;
		u32 cur_pos;

		u32 cycles_ahead = 0;
		u32 sequencer_cycles = 0;
		u32 sequencer_frame = 0;
		u8 dummy_regs[2] = {};

		bool enabled = false;
		bool double_speed = false;

		void step_ahead(u32 cycles);

	public:
		APU();

		//TODO: refactor it, test impl
		void test_attach(function<u8**(u8**, u32)> sbc, function<void(bool)> eac, u8** buffers)
		{
			swap_buffers_callback = sbc;
			enable_audio_callback = eac;
			sample_buffers = buffers;
			cur_pos = 0;
		}
		
		u8 read_byte(u16 adress, u32 cycles_passed) override;
		void write_byte(u16 adress, u8 value, u32 cycles_passed) override;

		void step(u32 cycles);
		void set_speed(bool speed) { double_speed = speed; }

		void serialize(std::ostream& stream);
		void deserialize(std::istream& stream);
};
