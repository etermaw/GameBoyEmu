#pragma once

#include "core/core.h"

class Tester
{
	private:
		function<u32*()> render_callback; //real renderer callback
		std::unique_ptr<u32[]> frame_buffer;
		std::unique_ptr<u8[]> internal_buffer;

		u32* frame_buffer_ptr = nullptr;
		u8* dummy_buffers[4];

		bool use_renderer = false;
		bool calculate_hash = false;
		bool running = true;

	public:
		Tester();

		//apu
		void dummy(bool unused);
		u8** swap_buffers(u8** ptr, u32 unused);

		//gpu
		u32* draw_frame();

		//gui
		bool is_running() const;
		void pump_input(Core& emu_core);

		void attach_renderer(function<u32*()> callback);
};