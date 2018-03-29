#pragma once
#include "stdafx.h"
#include "sha256.h"
#include "core/joypad.h"

class Tester
{
	private:
		bool calculate_hash = false;
		u8* dummy_ptrs[4];

	public:
		Tester();
		~Tester();

		bool input_stub(Joypad& input);
		void render_stub(const u32* frame_buffer);
		void audio_dummy_ctrl(bool unused);
		u8** audio_dummy_swap(u8** ptr, u32 unused);
};
