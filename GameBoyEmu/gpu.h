#pragma once
#include "stdafx.h"
#include "IMemory.h"
#include "interrupts.h"

class Gpu final : public IMemory
{
	private:
		Interrupts& interrupts;
		
		std::unique_ptr<u32[]> screen_buffer;
		std::unique_ptr<u8[]> vram[2]; 
		std::unique_ptr<u8[]> oam;
		const u8* ram_ptr;

		u32 cycles_ahead;
		u32 cycles;
		i32 dma_cycles;
		i32 enable_delay;

		u8 regs[12];

		u16 cgb_bgp[8];
		u16 cgb_obp[8];
		u8 hdma_regs[5];
		u8 vram_bank;

		bool entering_vblank;
		bool cgb_mode;

		void vb_mode();
		void hb_mode();
		void oam_mode();
		void transfer_mode();

		void dma_copy(u8 adress);

		void draw_background_row(); //DMG 
		void draw_sprite_row(); //DMG
		void draw_window_row(); //DMG
		void draw_line();

		void turn_off_lcd();
		void turn_on_lcd();

		void step_ahead(u32 cycles);

	public:
		Gpu(Interrupts& ints);

		u8 read_byte(u16 adress, u32 cycles_passed) override;
		void write_byte(u16 adress, u8 value, u32 cycles_passed) override;

		bool is_entering_vblank();
		
		void step(u32 clock_cycles);
		const u32* get_frame_buffer() const { return screen_buffer.get(); }

		void set_ram_dma(const u8* src) { ram_ptr = src; }
		void enable_cgb_mode(bool enable) { cgb_mode = enable; }
};