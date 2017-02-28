#pragma once
#include "stdafx.h"
#include "IMemory.h"
#include "interrupts.h"

class Gpu final : public IMemory
{
	private:
		std::function<u8*(u16)> memory_callback;
		std::unique_ptr<u32[]> screen_buffer;

		std::unique_ptr<u8[]> vram;//[2]; 
		
		u32 cycles;
		i32 dma_cycles;
		i32 enable_delay;

		u8 oam[0xA0];
		u8 regs[12];

		void vb_mode(Interrupts& interrupts);
		void hb_mode(Interrupts& interrupts);
		void oam_mode(Interrupts& interrupts);
		void transfer_mode(Interrupts& interrupts);

		void dma_copy(u8 adress);

		void draw_background_row();
		void draw_sprite_row();
		void draw_window_row();
		void draw_line();

		void turn_off_lcd();
		void turn_on_lcd();

	public:
		Gpu();

		u8 read_byte(u16 adress) override;
		void write_byte(u16 adress, u8 value) override;
		
		u32 step(u32 clock_cycles, Interrupts& interrupts);
		const u32* get_frame_buffer() const { return screen_buffer.get(); }
		void set_memory_callback(std::function<u8*(u16)> callback) { memory_callback = callback; }
};