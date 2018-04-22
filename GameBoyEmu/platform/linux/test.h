#pragma once
#include "stdafx.h"
#include "sha256.h"
#include "core/joypad.h"

class Tester
{
	private:
		function<void(const u32*)> render_callback; //real renderer callback
		bool use_renderer = false;
		bool calculate_hash = false;
		u8* dummy_ptrs[4];

	public:
		Tester();
		~Tester();

		bool input_stub(Joypad& input);

		void attach_renderer(function<void(const u32*)> callback);
};

//test stubs, for compability

class AudioTEST
{
	private:

	public:
		void dummy(bool unused);
		u8** swap_buffers(u8** ptr, u32 unused);

		void attach_impl(std::shared_ptr<Tester> u);
};

class RendererTEST
{
	private:
		std::shared_ptr<Tester> pimpl;

	public:
		RendererTEST(void* ptr);

		void vblank_handler(const u32* frame_buffer);
		void attach_impl(std::shared_ptr<Tester> u);
};

class GuiTEST
{
	private:
		std::shared_ptr<Tester> pimpl;

	public:
		GuiTEST(u32 u1, u32 u2, const std::string& u3);

		void* get_display();
		bool is_running() const;
		void pump_input(Core& emu_core);
		void attach_impl(std::shared_ptr<Tester> u);
};
