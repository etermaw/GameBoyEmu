#pragma once
#include "stdafx.h"
#include "IMemory.h"
#include "interrupts.h"

class Gpu final : public IMemory
{
	enum GPU_STATE { GS_VBLANK, GS_LY_153, GS_LY_153_0, GS_HBLANK, GS_OAM, GS_TRANSFER_PREFETCHING, GS_TRANSFER_DRAWING, GS_LCD_OFF, GS_TURNING_ON };

	struct oam_entry
	{
		u8 x;
		u8 y;
		u8 tile_num;
		u8 atr;

		bool operator< (const oam_entry& other) const
		{
			return x < other.x;
		}
	};

	private:
		Interrupts& interrupts;
		
		std::unique_ptr<u32[]> screen_buffer;
		std::unique_ptr<u8[]> vram[2]; 
		std::unique_ptr<u8[]> oam;
		IDmaMemory* cart;
		IDmaMemory* ram;

		GPU_STATE current_state;
		u32 cycles_to_next_state;

		u32 cycles_ahead = 0;
		u32 cycles = 0;
		i32 dma_cycles = 0;
		i32 enable_delay = 0;

		u8 regs[12] = {}; //TODO: initial values

		u32 current_pixels_drawn = 0;
		oam_entry sorted_sprites[10] = {};
		i32 sprite_count = 0;

		std::bitset<160> priority_buffer;
		std::bitset<160> alpha_buffer;
		u32 hdma_cur = 0;
		u32 new_dma_cycles = 0;
		
		u8 cgb_bgp[64];
		u8 cgb_obp[64];

		u32 color_bgp[8][4];
		u32 color_obp[8][4];
		u8 cgb_bgp_index = 0;
		u8 cgb_obp_index = 0;
		u8 hdma_regs[5]; //TODO: inital values?
		u8 vram_bank = 0;
		
		bool entering_vblank = false;
		bool cgb_mode = false;
		bool cgb_bgp_autoinc = false;
		bool cgb_obp_autoinc = false;
		bool hdma_active = false;
		bool double_speed = false;
		bool unlocked_vram = true;
		bool unlocked_oam = true;

		void vb_mode();
		void hb_mode();
		void oam_mode();
		void transfer_mode();

		void launch_dma(u8 adress);
		void launch_gdma();
		void launch_hdma();

		void prepare_sprites();

		void draw_background_row(u32 start, u32 end); //DMG
		void draw_window_row(u32 start, u32 end); //DMG
		void draw_sprite_row(u32 start, u32 end); //DMG

		void draw_background_row_cgb(u32 start, u32 end);
		void draw_window_row_cgb(u32 start, u32 end);
		void draw_sprite_row_cgb(u32 start, u32 end);

		void draw_line(u32 pixel_start, u32 pixel_end);

		void turn_off_lcd();
		void turn_on_lcd();

		const u8* resolve_adress(u16 adress) const;
		
		void step_ahead(u32 cycles);

		void get_gpu_status(std::array<u8, 12>& dmg, std::array<u8, 8>& cgb)
		{
			std::memcpy(dmg.data(), regs, sizeof(u8) * 12);

			dmg[1] = change_bit(set_bit(dmg[1], 7), dmg[4] == dmg[5], 2);

			std::memcpy(cgb.data(), hdma_regs, sizeof(u8) * 5);
			cgb[5] = cgb_bgp_index;
			cgb[6] = cgb_obp_index;
			cgb[7] = vram_bank;
		}

	public:
		Gpu(Interrupts& ints);

		u8 read_byte(u16 adress, u32 cycles_passed) override;
		void write_byte(u16 adress, u8 value, u32 cycles_passed) override;

		bool is_entering_vblank();
		
		u32 step(u32 clock_cycles);
		const u32* get_frame_buffer() const { return screen_buffer.get(); }
		void clear_frame_buffer() { std::memset(screen_buffer.get(), 0xFF, sizeof(u32) * 144 * 160); }

		void enable_cgb_mode(bool enable) { cgb_mode = enable; }
		void set_speed(bool speed) { double_speed = speed; }

		void attach_dma_ptrs(IDmaMemory* cart_memory, IDmaMemory* ram_memory)
		{
			cart = cart_memory;
			ram = ram_memory;
		}

		void serialize(std::ostream& stream);
		void deserialize(std::istream& stream);

		function<void(std::array<u8, 12>&, std::array<u8, 8>&)> get_debug_func() { return make_function(&Gpu::get_gpu_status, this); }
};
