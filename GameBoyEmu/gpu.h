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

		std::bitset<160> priority_buffer;
		u32 hdma_cur;
		u32 new_dma_cycles;
		u32 color_bgp[8][4];
		u32 color_obp[8][4];
		u8 cgb_bgp_index;
		u8 cgb_obp_index;
		u8 hdma_regs[5];
		u8 vram_bank;
		
		bool entering_vblank;
		bool cgb_mode;
		bool cgb_bgp_autoinc;
		bool cgb_obp_autoinc;
		bool hdma_active;
		bool double_speed;

		void vb_mode();
		void hb_mode();
		void oam_mode();
		void transfer_mode();

		void launch_dma(u8 adress);
		void launch_gdma();
		void launch_hdma();

		void draw_background_row(); //DMG 
		void draw_sprite_row(); //DMG
		void draw_window_row(); //DMG

		void draw_background_row_cgb();
		void draw_window_row_cgb();
		void draw_sprite_row_cgb();

		void draw_line();

		void turn_off_lcd();
		void turn_on_lcd();

		const u8* resolve_adress(u16 adress) const;
		
		void step_ahead(u32 cycles);

	public:
		Gpu(Interrupts& ints);

		u8 read_byte(u16 adress, u32 cycles_passed) override;
		void write_byte(u16 adress, u8 value, u32 cycles_passed) override;

		bool is_entering_vblank();
		
		u32 step(u32 clock_cycles);
		const u32* get_frame_buffer() const { return screen_buffer.get(); }

		void enable_cgb_mode(bool enable) { cgb_mode = enable; }
		void set_speed(bool speed) { double_speed = speed; }

		void attach_dma_ptrs(const u8* cart_rom, const u8* cart_ram, const u8* internal_ram)
		{
			ram_ptr = internal_ram;
		}
};