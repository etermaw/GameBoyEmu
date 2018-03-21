#include "gpu.h"

enum IO_REGS {
	IO_LCD_CONTROL, IO_LCD_STATUS, IO_SY, IO_SX, IO_LY, IO_LYC,
	IO_DMA, IO_BGP, IO_OBP_0, IO_OBP_1, IO_WY, IO_WX7
};

enum LCD_CONTROL {
	LC_BG_ENABLED, LC_SPRITES_ENABLED, LC_SPRITES_SIZE, LC_BG_TMAP,
	LC_TILESET, LC_WINDOW_ENABLED, LC_WINDOW_TMAP, LC_POWER
};

enum LCD_STATUS { LS_CMP_SIG = 2, LS_HBLANK, LS_VBLANK, LS_OAM, LS_LYC_LY };

template<class T>
inline T clamp(T value, T min, T max)
{
	return std::min(max, std::max(value, min));
}

u32 get_dmg_color(u32 num)
{
	static const u32 color_tab[4] = { 0xFFFFFFFF, 0xFFAAAAAA, 0xFF555555, 0xFF000000 };

	assert(num < 4);
	return color_tab[num]; //0xFFFFFFFF - (0x00555555 * num);
}

u32 cgb_to_rgb(u16 cgb)
{
	u32 r = cgb & 0x1F;
	u32 g = (cgb >> 5) & 0x1F;
	u32 b = (cgb >> 10) & 0x1F;

	r = ((r * 527 + 23) >> 6) & 0xFF;
	g = ((g * 527 + 23) >> 6) & 0xFF;
	b = ((b * 527 + 23) >> 6) & 0xFF;

	return 0xFF000000 | (r << 16) | (g << 8) | b;
}

Gpu::Gpu(Interrupts& ints) : interrupts(ints)
{
	regs[IO_LCD_CONTROL] = 0x91;
	regs[IO_BGP] = 0xFC;
	regs[IO_OBP_0] = 0xFF;
	regs[IO_OBP_1] = 0xFF;

	vram[0] = std::make_unique<u8[]>(0x2000);
	vram[1] = std::make_unique<u8[]>(0x2000);
	screen_buffer = std::make_unique<u32[]>(144 * 160);
	oam = std::make_unique<u8[]>(0xA0);

	std::memset(screen_buffer.get(), 0xFF, sizeof(u32) * 144 * 160);
	std::memset(oam.get(), 0xFF, sizeof(u8) * 0xA0);

	std::memset(cgb_bgp, 0xFF, sizeof(cgb_bgp));
	std::memset(cgb_obp, 0xFF, sizeof(cgb_obp));
	std::memset(color_bgp, 0xFF, sizeof(u32) * 8 * 4);
	std::memset(color_obp, 0xFF, sizeof(u32) * 8 * 4);

	current_state = GS_HBLANK;
	cycles_to_next_state = 204; //TODO: what`s GPU status immediately after BIOS launch?
}

void Gpu::vb_mode()
{
	regs[IO_LY]++;

	if (check_bit(regs[IO_LCD_STATUS], LS_LYC_LY) && regs[IO_LY] == regs[IO_LYC])
		interrupts.raise(INT_LCD);

	if (regs[IO_LY] < 153)
		cycles_to_next_state = 456;

	else
	{
		current_state = GS_LY_153;
		cycles_to_next_state = 4;
	}
}

void Gpu::hb_mode()
{
	regs[IO_LY]++;

	if (check_bit(regs[IO_LCD_STATUS], LS_LYC_LY) && regs[IO_LY] == regs[IO_LYC])
		interrupts.raise(INT_LCD);

	if (regs[IO_LY] < 144)
	{
		regs[IO_LCD_STATUS] = (regs[IO_LCD_STATUS] & 0xFC) | 0x2; //OAM mode
		unlocked_oam = false;

		if (check_bit(regs[IO_LCD_STATUS], LS_OAM))
			interrupts.raise(INT_LCD);

		current_state = GS_OAM;
		cycles_to_next_state = 80;
	}

	else
	{
		interrupts.raise(INT_VBLANK);

		if (check_bit(regs[IO_LCD_STATUS], LS_VBLANK))
			interrupts.raise(INT_LCD);

		regs[IO_LCD_STATUS] = (regs[IO_LCD_STATUS] & 0xFC) | 0x1; //VBLANK mode
		entering_vblank = true;

		current_state = GS_VBLANK;
		cycles_to_next_state = 456;
	}
}

void Gpu::oam_mode()
{
	regs[IO_LCD_STATUS] = (regs[IO_LCD_STATUS] & 0xFC) | 0x3; //transfer mode
	unlocked_vram = false;

	current_state = GS_TRANSFER_PREFETCHING;
	cycles_to_next_state = 6;

	prepare_sprites();
	current_pixels_drawn = 0;
}

void Gpu::transfer_mode()
{
	if (current_pixels_drawn < 160U)
		draw_line(current_pixels_drawn, 160U);

	if (check_bit(regs[IO_LCD_STATUS], LS_HBLANK))
		interrupts.raise(INT_LCD);

	regs[IO_LCD_STATUS] &= 0xFC; //HBLANK mode
	unlocked_oam = true;
	unlocked_vram = true;

	if (hdma_active)
		launch_hdma();

	current_state = GS_HBLANK;
	cycles_to_next_state = 204 - (regs[IO_SX] % 8);
}

void Gpu::step_ahead(u32 clock_cycles)
{
	if (dma_cycles > 0)
		dma_cycles = std::max(0, dma_cycles - static_cast<i32>(clock_cycles << double_speed));

	if (current_state == GS_LCD_OFF)
		return;

    cycles += clock_cycles;

	while (cycles >= cycles_to_next_state)
	{
		cycles -= cycles_to_next_state;

		switch (current_state)
		{
			case GS_VBLANK:
				vb_mode();
				break;

			case GS_LY_153:
				regs[IO_LY] = 0;

				if (check_bit(regs[IO_LCD_STATUS], LS_LYC_LY) && regs[IO_LY] == regs[IO_LYC])
					interrupts.raise(INT_LCD);

				current_state = GS_LY_153_0;
				cycles_to_next_state = 456 - 4;
				break;

			case GS_LY_153_0:
				regs[IO_LCD_STATUS] = (regs[IO_LCD_STATUS] & 0xFC) | 0x2; //OAM mode
				unlocked_oam = false;

				if (check_bit(regs[IO_LCD_STATUS], LS_OAM))
					interrupts.raise(INT_LCD);

				current_state = GS_OAM;
				cycles_to_next_state = 80;
				break;

			case GS_HBLANK:
				hb_mode();
				break;

			case GS_OAM:
				oam_mode();
				break;

			case GS_TRANSFER_PREFETCHING:
				current_state = GS_TRANSFER_DRAWING;
				cycles_to_next_state = 172 - 6 + (regs[IO_SX] % 8);
				break;

			case GS_TRANSFER_DRAWING:
				transfer_mode();
				break;

			case GS_TURNING_ON:
				current_state = GS_HBLANK;
				cycles_to_next_state = 204;
				break;
		}
	}
}

void Gpu::launch_dma(u8 adress)
{
	const u8* src_ptr = resolve_adress(adress * 0x100);
	
	if (src_ptr)
		std::memcpy(oam.get(), src_ptr, sizeof(u8) * 0xA0);

	else
		std::memset(oam.get(), 0xFF, sizeof(u8) * 0xA0);

	dma_cycles = 648;
	//TODO: add memory bus conficts if cpu don`t operate on HRAM during oam dma?
	//cpu should read last word read by dma (require messing with MMU)
}

void Gpu::launch_gdma()
{
	u16 src = (hdma_regs[0] << 8) | (hdma_regs[1] & 0xF0);
	u16 dst = ((hdma_regs[2] & 0x1F) << 8) | (hdma_regs[3] & 0xF0); //0x0000 - 0x1FF0
	u16 len = ((hdma_regs[4] & 0x7F) + 1) * 0x10;

	//TODO: max len is 0x800, so it can overlap 2 mem regions. Detect it and get 2nd ptr
	//TODO: what if dst + len is bigger then vram?
	//TODO: edge case, what if someone launch GMDA not in vblank state? emulate no access
	const u8* src_ptr = resolve_adress(src);

	if (src_ptr)
		std::memcpy(&vram[vram_bank][dst], src_ptr, sizeof(u8) * len);

	else
		std::memset(&vram[vram_bank][dst], 0xFF, sizeof(u8) * len);

	new_dma_cycles = len * 2;
	hdma_regs[4] = 0xFF;
}

void Gpu::launch_hdma()
{
	u16 src = (hdma_regs[0] << 8) | (hdma_regs[1] & 0xF0);
	u16 dst = ((hdma_regs[2] & 0x1F) << 8) | (hdma_regs[3] & 0xF0); //0x0000 - 0x1FF0
	u8 len = (hdma_regs[4] & 0x7F) + 1;
	u16 cur_pos = hdma_cur * 0x10;

	const u8* src_ptr = resolve_adress(src + cur_pos);

	if (src_ptr)
		std::memcpy(&vram[vram_bank][dst + cur_pos], src_ptr, sizeof(u8) * 0x10);

	else
		std::memset(&vram[vram_bank][dst + cur_pos], 0xFF, sizeof(u8) * 0x10);

	if (--len == 0)
	{
		hdma_active = false;
		hdma_regs[4] = 0xFF;
	}

	else
	{
		++hdma_cur;
		hdma_regs[4] = clear_bit(len - 1, 7);
	}

	new_dma_cycles = 32;
}

void Gpu::prepare_sprites()
{ 
	const i32 line = regs[IO_LY];
	const i32 height = check_bit(regs[IO_LCD_CONTROL], LC_SPRITES_SIZE) ? 16 : 8;

    sprite_count = 0;

	for (u32 i = 0; i < 40 && sprite_count < 10; ++i)
	{
		i32 y = oam[i * 4] - 16;

		if (y <= line && (y + height) > line)
		{
			sorted_sprites[sprite_count].y = oam[i * 4];
			sorted_sprites[sprite_count].x = oam[i * 4 + 1];
			sorted_sprites[sprite_count].tile_num = oam[i * 4 + 2];
			sorted_sprites[sprite_count].atr = oam[i * 4 + 3];

			++sprite_count;
		}
	}

	if (!cgb_mode)
		std::stable_sort(std::begin(sorted_sprites), std::next(std::begin(sorted_sprites), sprite_count));
}

void Gpu::draw_background_row(u32 start, u32 end)
{
	const u32 sy = regs[IO_SY];
	const u32 sx = regs[IO_SX];
	const u32 line = regs[IO_LY];
	
	const u32 offset = check_bit(regs[IO_LCD_CONTROL], LC_BG_TMAP) ? 0x1C00 : 0x1800; //0x9C00,0x9800
	const u32 data_offset = check_bit(regs[IO_LCD_CONTROL], LC_TILESET) ? 0 : 0x800; //0x8000,0x8800
	const u32 index_corrector = check_bit(regs[IO_LCD_CONTROL], LC_TILESET) ? 0 : 128;
	const u32 buffer_offset = line * 160;
	
	const u8* tile_nums = &vram[0][offset]; 
	const u8* tile_data = &vram[0][data_offset]; //index_corrector * 16 == data_offset

	const u32 line_offset = (((line + sy) / 8) % 32) * 32;
	const u32 tile_line = (line + sy) % 8;

	for (u32 i = start; i < end;) 
	{
		//it should work as singed/unsigned u8
		auto tile_num = (tile_nums[line_offset + (((sx + i) / 8) % 32)] + index_corrector) & 0xFF;

		u8 tile_low = tile_data[tile_num * 16 + tile_line * 2]; 
		u8 tile_high = tile_data[tile_num * 16 + tile_line * 2 + 1];

		for (u32 j = (sx + i) % 8; j < 8 && i < end; ++j, ++i)
		{
			u32 id = 7 - j;
			u32 color_id = (check_bit(tile_high, id) << 1) | check_bit(tile_low, id);
			u32 shade_num = (regs[IO_BGP] >> (color_id * 2)) & 0x3;

			screen_buffer[buffer_offset + i] = get_dmg_color(shade_num);
		}
	}
}

void Gpu::draw_window_row(u32 start, u32 end)
{
	if (regs[IO_LY] < regs[IO_WY] || regs[IO_WX7] > 166 || end <= regs[IO_WX7])
		return;

	const u32 line = regs[IO_LY];
	const u32 wy = regs[IO_WY];
	const u32 wx = std::max(0, static_cast<i32>(regs[IO_WX7]) - 7);

	const u32 offset = check_bit(regs[IO_LCD_CONTROL], LC_WINDOW_TMAP) ? 0x1C00 : 0x1800; //0x9C00,0x9800
	const u32 data_offset = check_bit(regs[IO_LCD_CONTROL], LC_TILESET) ? 0 : 0x800; //0x8000,0x8800
	const u32 index_corrector = check_bit(regs[IO_LCD_CONTROL], LC_TILESET) ? 0 : 128;
	const u32 buffer_offset = line * 160;

	const u8* tile_nums = &vram[0][offset];
	const u8* tile_data = &vram[0][data_offset]; //data_offset == index_corrector * 16

	const u32 window_line = line - wy;
	const u32 tile_line = window_line % 8;
	const u32 line_off = (window_line / 8) * 32;

	for (u32 i = std::max(start, wx); i < end;)
	{
		const u32 tile_num_pos = i - wx;

		u32 tile_num = (tile_nums[line_off + tile_num_pos / 8] + index_corrector) & 0xFF;
		u8 tile_low = tile_data[tile_num * 16 + tile_line * 2];
		u8 tile_high = tile_data[tile_num * 16 + tile_line * 2 + 1];

		for (u32 j = tile_num_pos % 8; j < 8 && i < end; ++j, ++i)
		{
			u32 id = 7 - j;
			u32 color_id = (check_bit(tile_high, id) << 1) | check_bit(tile_low, id);
			u32 shade_num = (regs[IO_BGP] >> (color_id * 2)) & 0x3;

			screen_buffer[buffer_offset + i] = get_dmg_color(shade_num);
		}
	}
}

void Gpu::draw_sprite_row(u32 pixel_start, u32 pixel_end)
{	
	const u8* tile_data = &vram[0][0]; //0x8000
	const bool sprite_size = check_bit(regs[IO_LCD_CONTROL], LC_SPRITES_SIZE);
	const i32 line = regs[IO_LY];
	const i32 height = sprite_size ? 16 : 8;
	const u32 line_offset = line * 160;
	const u32 bg_alpha_color = get_dmg_color(regs[IO_BGP] & 0x3);

	for (i32 i = sprite_count - 1; i >= 0; --i)
	{
		i32 sx = sorted_sprites[i].x - 8;
		i32 sy = sorted_sprites[i].y - 16;
		u32 tile_num = sorted_sprites[i].tile_num;
		u32 atr = sorted_sprites[i].atr;

		u32 tile_line = line - sy;
		const u32 palette_num = IO_OBP_0 + check_bit(atr, 4); //0 - OBP[0], 1 - OBP[1]

		if (check_bit(atr, 6)) //Y flip
			tile_line = height - tile_line - 1;
				
		if (sprite_size)
			tile_num = tile_line < 8 ? (tile_num & 0xFE) : (tile_num | 0x1);
		
		tile_line %= 8;

		u8 tile_low = tile_data[tile_num * 16 + tile_line * 2];
		u8 tile_high = tile_data[tile_num * 16 + tile_line * 2 + 1];

		if (check_bit(atr, 5)) //X flip
		{
			tile_low = flip_bits(tile_low);
			tile_high = flip_bits(tile_high);
		}

		const u32 begin = std::max(static_cast<i32>(pixel_start), sx);
		const u32 end = std::min(sx + 8, static_cast<i32>(pixel_end));
		const u32 sprite_end = sx + 8;

		for (u32 j = begin; j < end; ++j)
		{
			u32 id = sprite_end - j - 1;
			u32 color_id = (check_bit(tile_high, id) << 1) | check_bit(tile_low, id);
			u32 color = get_dmg_color((regs[palette_num] >> (color_id * 2)) & 0x3);

			if (color_id != 0)
			{
				if (check_bit(atr, 7)) //BG has priority
				{
					if (screen_buffer[line_offset + j] == bg_alpha_color)
						screen_buffer[line_offset + j] = color;
				}

				else //sprite has priority
					screen_buffer[line_offset + j] = color;
			}
		}
	}
}

void Gpu::draw_background_row_cgb(u32 start, u32 end)
{
	const u32 sy = regs[IO_SY];
	const u32 sx = regs[IO_SX];
	const u32 line = regs[IO_LY];

	const u32 offset = check_bit(regs[IO_LCD_CONTROL], LC_BG_TMAP) ? 0x1C00 : 0x1800; //0x9C00,0x9800
	const u32 data_offset = check_bit(regs[IO_LCD_CONTROL], LC_TILESET) ? 0 : 0x800; //0x8000,0x8800
	const u32 index_corrector = check_bit(regs[IO_LCD_CONTROL], LC_TILESET) ? 0 : 128;
	const u32 buffer_offset = line * 160;

	const u8* tile_nums = &vram[0][offset];
	const u8* tile_atrs = &vram[1][offset];

	const u8* tile_data[2] = { &vram[0][data_offset], &vram[1][data_offset] };

	const u32 screen_line = (((line + sy) / 8) % 32) * 32; //which line in "screen" (tile is 8x8 pixels, that`s why we div by 8)
	const u32 row_tile_line = (line + sy) % 8;

	for (u32 i = start; i < end;)
	{
		//it should work as singed/unsigned u8
		auto tile_num = (tile_nums[screen_line + (((sx + i) / 8) % 32)] + index_corrector) & 0xFF;
		auto tile_atr = tile_atrs[screen_line + (((sx + i) / 8) % 32)];

		u8 palette_num = tile_atr & 0x7;
		u8 data_bank = check_bit(tile_atr, 3);
		bool priority = check_bit(tile_atr, 7);
        auto tile_line = row_tile_line;        

		if (check_bit(tile_atr, 6)) //Y flip
			tile_line = 7 - tile_line;

		u8 tile_low = tile_data[data_bank][tile_num * 16 + tile_line * 2];
		u8 tile_high = tile_data[data_bank][tile_num * 16 + tile_line * 2 + 1];

		if (check_bit(tile_atr, 5)) //X flip
		{
			tile_low = flip_bits(tile_low);
			tile_high = flip_bits(tile_high);
		}

		for (u32 j = (sx + i) % 8; j < 8 && i < end; ++j, ++i)
		{
			u32 id = 7 - j;
			u32 color_id = (check_bit(tile_high, id) << 1) | check_bit(tile_low, id);

			screen_buffer[buffer_offset + i] = color_bgp[palette_num][color_id];
			priority_buffer[i] = priority;
			alpha_buffer[i] = (color_id != 0);
		}
	}
}

void Gpu::draw_window_row_cgb(u32 start, u32 end)
{
	if (regs[IO_LY] < regs[IO_WY] || regs[IO_WX7] > 166)
		return;

	const u32 line = regs[IO_LY];
	const u32 wy = regs[IO_WY];
	const u32 wx = std::max(0, static_cast<i32>(regs[IO_WX7]) - 7);

	const u32 offset = check_bit(regs[IO_LCD_CONTROL], LC_WINDOW_TMAP) ? 0x1C00 : 0x1800; //0x9C00,0x9800
	const u32 data_offset = check_bit(regs[IO_LCD_CONTROL], LC_TILESET) ? 0 : 0x800; //0x8000,0x8800
	const u32 index_corrector = check_bit(regs[IO_LCD_CONTROL], LC_TILESET) ? 0 : 128;
	const u32 buffer_offset = line * 160;

	const u8* tile_nums = &vram[0][offset];
	const u8* tile_atrs = &vram[1][offset];
	const u8* tile_data[2] = { &vram[0][data_offset], &vram[1][data_offset] };
	
	const u32 window_line = line - wy;
	const u32 row_tile_line = window_line % 8;
	const u32 line_offset = (window_line / 8) * 32;

	for (u32 i = std::max(start, wx); i < end;)
	{
		const u32 tile_num_pos = i - wx;

		u32 tile_num = (tile_nums[line_offset + tile_num_pos / 8] + index_corrector) & 0xFF;
		u8 tile_atr = tile_atrs[line_offset + tile_num_pos / 8];

		u8 palette_num = tile_atr & 0x7;
		u8 data_bank = check_bit(tile_atr, 3);
		bool priority = check_bit(tile_atr, 7);
        auto tile_line = row_tile_line;        

		if (check_bit(tile_atr, 6)) //Y flip
			tile_line = 7 - tile_line;

		u8 tile_low = tile_data[data_bank][tile_num * 16 + tile_line * 2];
		u8 tile_high = tile_data[data_bank][tile_num * 16 + tile_line * 2 + 1];

		if (check_bit(tile_atr, 5)) //X flip
		{
			tile_low = flip_bits(tile_low);
			tile_high = flip_bits(tile_high);
		}

		for (u32 j = tile_num_pos % 8; j < 8 && i < end; ++j, ++i)
		{
			u32 id = 7 - j;
			u32 color_id = (check_bit(tile_high, id) << 1) | check_bit(tile_low, id);

			screen_buffer[buffer_offset + i] = color_bgp[palette_num][color_id];
			priority_buffer[i] = priority;
			alpha_buffer[i] = (color_id != 0);
		}
	}
}

//priorities for GBC:  BG0 < OBJL < BGL < OBJH < BGH
void Gpu::draw_sprite_row_cgb(u32 pixel_start, u32 pixel_end)
{
	const u8* tile_data[2] = { &vram[0][0], &vram[1][0] }; //0x8000
	const bool sprite_size = check_bit(regs[IO_LCD_CONTROL], LC_SPRITES_SIZE);
	const i32 line = regs[IO_LY];
	const i32 height = sprite_size ? 16 : 8;
	const u32 line_offset = line * 160;

	for (i32 i = sprite_count - 1; i >= 0; --i)
	{
		i32 sx = sorted_sprites[i].x - 8;
		i32 sy = sorted_sprites[i].y - 16;
		u32 tile_num = sorted_sprites[i].tile_num;
		u32 atr = sorted_sprites[i].atr;

		u32 tile_line = line - sy;
		const u32 palette_num = atr & 0x7;
		const u32 bank_num = check_bit(atr, 3);

		if (check_bit(atr, 6)) //Y flip
			tile_line = height - tile_line - 1;

		if (sprite_size)
			tile_num = tile_line < 8 ? (tile_num & 0xFE) : (tile_num | 0x1);

		tile_line %= 8;

		u8 tile_low = tile_data[bank_num][tile_num * 16 + tile_line * 2];
		u8 tile_high = tile_data[bank_num][tile_num * 16 + tile_line * 2 + 1];

		if (check_bit(atr, 5)) //X flip
		{
			tile_low = flip_bits(tile_low);
			tile_high = flip_bits(tile_high);
		}

		const u32 begin = std::max(static_cast<i32>(pixel_start), sx);
		const u32 end = std::min(sx + 8, static_cast<i32>(pixel_end));
		const u32 sprite_end = sx + 8;

		for (u32 j = begin; j < end; ++j)
		{
			u32 id = sprite_end - j - 1;
			u32 color_id = (check_bit(tile_high, id) << 1) | check_bit(tile_low, id);
			u32 color = color_obp[palette_num][color_id];

			if (color_id != 0)
			{
				if (priority_buffer[j]) //if BG prior == 1, then sprite will cover only BG 0 color
				{
					if (!alpha_buffer[j])
						screen_buffer[line_offset + j] = color;
				}

				else
				{
					if (check_bit(atr, 7))
					{
						if (!alpha_buffer[j])
							screen_buffer[line_offset + j] = color;
					}

					else
						screen_buffer[line_offset + j] = color;
				}
			}

		}
	}
}

void Gpu::draw_line(u32 pixel_start, u32 pixel_end)
{
	if (check_bit(regs[IO_LCD_CONTROL], LC_POWER))
	{
		if (cgb_mode)
		{
			priority_buffer.reset();
			alpha_buffer.reset();
			
			//in CGB, background is always visible
			draw_background_row_cgb(pixel_start, pixel_end);

			if (check_bit(regs[IO_LCD_CONTROL], LC_WINDOW_ENABLED))
				draw_window_row_cgb(pixel_start, pixel_end);

			//if background is "disabled", background & window lose priority
			if (!check_bit(regs[IO_LCD_CONTROL], LC_BG_ENABLED))
				priority_buffer.reset();

			if (check_bit(regs[IO_LCD_CONTROL], LC_SPRITES_ENABLED))
				draw_sprite_row_cgb(pixel_start, pixel_end);
		}

		else
		{
			if (check_bit(regs[IO_LCD_CONTROL], LC_BG_ENABLED))
				draw_background_row(pixel_start, pixel_end);

			if (check_bit(regs[IO_LCD_CONTROL], LC_WINDOW_ENABLED))
				draw_window_row(pixel_start, pixel_end);

			if (check_bit(regs[IO_LCD_CONTROL], LC_SPRITES_ENABLED))
				draw_sprite_row(pixel_start, pixel_end);
		}
	}
}

void Gpu::turn_off_lcd()
{
	std::memset(screen_buffer.get(), 0xFF, sizeof(u32) * 160 * 140);
	regs[IO_LY] = 0;
	regs[IO_LCD_STATUS] &= 0xFC; //HBLANK mode
	unlocked_oam = true;
	unlocked_vram = true;
	cycles = 0;

	entering_vblank = true; //display white screen
	current_state = GS_LCD_OFF;
	cycles_to_next_state = 0;
}

void Gpu::turn_on_lcd()
{
	current_state = GS_TURNING_ON;
	cycles_to_next_state = 240;
}

const u8* Gpu::resolve_adress(u16 adress) const
{
	if (adress < 0x8000)
		return cart->get_dma_ptr(adress); //ROM

	else if (adress >= 0x8000 && adress < 0xA000)
		return &vram[vram_bank][adress - 0x8000];

	else if (adress >= 0xA000 && adress < 0xC000)
		return cart->get_dma_ptr(adress); //RAM

	else if (adress >= 0xC000 && adress < 0xF000)
		return ram->get_dma_ptr(adress);

	else
		return nullptr;
}

u8 Gpu::read_byte(u16 adress, u32 cycles_passed)
{
	cycles_passed >>= double_speed;
	cycles_passed -= cycles_ahead;

	if (cycles_passed > 0)
	{
		step_ahead(cycles_passed);
		cycles_ahead += cycles_passed;
	}

	//if gpu is in mode 3 ignore read (return 0xFF instead)
	if (adress >= 0x8000 && adress < 0xA000)
		return unlocked_vram ? vram[vram_bank][adress - 0x8000] : 0xFF;

	//if gpu is in mode 2,3 or during oam dma, ignore read
	else if (adress >= 0xFE00 && adress < 0xFEA0)
		return (unlocked_oam && dma_cycles <= 0) ? oam[adress - 0xFE00] : 0xFF;

	else if (adress >= 0xFF40 && adress <= 0xFF4B)
	{
		if (adress != 0xFF41)
			return regs[adress - 0xFF40];

		else
			return change_bit(set_bit(regs[1], 7), regs[IO_LY] == regs[IO_LYC], LS_CMP_SIG);
	}

	else if (cgb_mode && adress == 0xFF4F)
		return vram_bank & 0xFE;

	else if (cgb_mode && adress >= 0xFF51 && adress < 0xFF55)
		return hdma_regs[adress - 0xFF51]; //TODO: mask out useless bits?

	else if (cgb_mode && adress == 0xFF55)
		return change_bit(hdma_regs[4], !hdma_active, 7);

	else if (cgb_mode && adress == 0xFF68)
		return change_bit(cgb_bgp_index, cgb_bgp_autoinc, 7);

	else if (cgb_mode && adress == 0xFF69)
		return unlocked_vram ? cgb_bgp[cgb_bgp_index] : 0xFF;

	else if (cgb_mode && adress == 0xFF6A)
		return change_bit(cgb_obp_index, cgb_obp_autoinc, 7);

	else if (cgb_mode && adress == 0xFF6B)
		return unlocked_vram ? cgb_obp[cgb_obp_index] : 0xFF;

	else
		return 0xFF;
}

void Gpu::write_byte(u16 adress, u8 value, u32 cycles_passed)
{
	cycles_passed >>= double_speed;
	cycles_passed -= cycles_ahead;

	if (cycles_passed > 0)
	{
		step_ahead(cycles_passed);
		cycles_ahead += cycles_passed;
	}

	//if gpu is in mode 3 ignore write
	if (adress >= 0x8000 && adress < 0xA000 && unlocked_vram)
		vram[vram_bank][adress - 0x8000] = value;

	//if gpu is in mode 2,3 or during oam dma, ignore write
	else if (adress >= 0xFE00 && adress < 0xFEA0 && unlocked_oam && dma_cycles <= 0)
		oam[adress - 0xFE00] = value;

	else if (adress >= 0xFF40 && adress <= 0xFF4B)
	{
		if ((adress == 0xFF42 || adress == 0xFF47) && current_state == GS_TRANSFER_DRAWING)
		{
			//lazy way: "cycles" are internal state cycle counter (that one used in step_ahead switch)
			draw_line(current_pixels_drawn, std::min(cycles, 160U));
			current_pixels_drawn = std::min(cycles, 160U);
		}

		if (adress == 0xFF40)
		{
			bool lcd_power = check_bit(value, LC_POWER);
			bool current_power = check_bit(regs[IO_LCD_CONTROL], LC_POWER);

			if (!lcd_power && current_power)
				turn_off_lcd();

			else if (lcd_power && !current_power)
				turn_on_lcd();

			regs[IO_LCD_CONTROL] = value;
		}

		//bits 0-2 in lcd_status are read only, so we need to mask them out!
		else if (adress == 0xFF41)
			regs[IO_LCD_STATUS] = (value & 0xF8) | (regs[IO_LCD_STATUS] & 0x3);

		else if (adress == 0xFF44)
		{
			//it`s read-only register
		}

		else if (adress == 0xFF46)
			launch_dma(value);

		else
			regs[adress - 0xFF40] = value;
	}

	else if (cgb_mode && adress == 0xFF4F)
		vram_bank = value & 1;

	else if (cgb_mode && adress >= 0xFF51 && adress < 0xFF55)
		hdma_regs[adress - 0xFF51] = value;

	else if (cgb_mode && adress == 0xFF55)
	{
		hdma_regs[4] = value;

		if (check_bit(value, 7))
		{
			hdma_cur = 0;
			hdma_active = true;
		}

		else
		{
			if (hdma_active)
				hdma_active = false;

			else
				launch_gdma();
		}		
	}

	else if (cgb_mode && adress == 0xFF68)
	{
		cgb_bgp_index = value & 0x3F;
		cgb_bgp_autoinc = check_bit(value, 7);
	}

	else if (cgb_mode && adress == 0xFF69 && unlocked_vram)
	{
		cgb_bgp[cgb_bgp_index] = value;
		
		u16 rgb15 = 0;

		if (cgb_bgp_index % 2 == 0)
			rgb15 = (cgb_bgp[cgb_bgp_index + 1] << 8) | value;

		else
			rgb15 = (value << 8) | cgb_bgp[cgb_bgp_index - 1];

		color_bgp[cgb_bgp_index / 8][(cgb_bgp_index % 8) / 2] = cgb_to_rgb(rgb15);

		if (cgb_bgp_autoinc)
			cgb_bgp_index = (cgb_bgp_index + 1) & 0x3F;
	}

	else if (cgb_mode && adress == 0xFF6A)
	{
		cgb_obp_index = value & 0x3F;
		cgb_obp_autoinc = check_bit(value, 7);
	}

	else if (cgb_mode && adress == 0xFF6B && unlocked_vram)
	{
		cgb_obp[cgb_obp_index] = value;
		
		u16 rgb15 = 0;

		if (cgb_obp_index % 2 == 0)
			rgb15 = (cgb_obp[cgb_obp_index + 1] << 8) | value;

		else
			rgb15 = (value << 8) | cgb_obp[cgb_obp_index - 1];

		color_obp[cgb_obp_index / 8][(cgb_obp_index % 8) / 2] = cgb_to_rgb(rgb15);

		if (cgb_obp_autoinc)
			cgb_obp_index = (cgb_obp_index + 1) & 0x3F;
	}
}

bool Gpu::is_entering_vblank()
{
	bool tmp = entering_vblank;
	entering_vblank = false;

	return tmp;
}

u32 Gpu::step(u32 clock_cycles)
{
	//TODO: dereferred HDMA cycles might cause trouble if we write 2 registers using one, 16bit write
	u32 cycles_passed = new_dma_cycles << double_speed;
	clock_cycles >>= double_speed;

	step_ahead(clock_cycles + new_dma_cycles - cycles_ahead);

	cycles_ahead = 0;
	new_dma_cycles = 0;

	return cycles_passed;
}

void Gpu::serialize(std::ostream& stream)
{
	stream.write(reinterpret_cast<char*>(vram[0].get()), sizeof(u8) * 0x2000);
	stream.write(reinterpret_cast<char*>(vram[1].get()), sizeof(u8) * 0x2000);
	stream.write(reinterpret_cast<char*>(oam.get()), sizeof(u8) * 0xA0);
	stream.write(reinterpret_cast<char*>(cgb_bgp), sizeof(u8) * 64);
	stream.write(reinterpret_cast<char*>(cgb_obp), sizeof(u8) * 64);
	stream.write(reinterpret_cast<char*>(regs), sizeof(u8) * 12);
	stream.write(reinterpret_cast<char*>(hdma_regs), sizeof(u8) * 5);

	stream << cycles_ahead << cycles << dma_cycles << enable_delay;
	stream << hdma_cur << new_dma_cycles << cgb_bgp_index << cgb_obp_index;
	stream << vram_bank << entering_vblank << cgb_mode << cgb_bgp_autoinc;
	stream << cgb_obp_autoinc << hdma_active << double_speed;
	stream << unlocked_vram << unlocked_oam;
}

void Gpu::deserialize(std::istream& stream)
{
	stream.read(reinterpret_cast<char*>(vram[0].get()), sizeof(u8) * 0x2000);
	stream.read(reinterpret_cast<char*>(vram[1].get()), sizeof(u8) * 0x2000);
	stream.read(reinterpret_cast<char*>(oam.get()), sizeof(u8) * 0xA0);
	stream.read(reinterpret_cast<char*>(cgb_bgp), sizeof(u8) * 64);
	stream.read(reinterpret_cast<char*>(cgb_obp), sizeof(u8) * 64);
	stream.read(reinterpret_cast<char*>(regs), sizeof(u8) * 12);
	stream.read(reinterpret_cast<char*>(hdma_regs), sizeof(u8) * 5);

	stream >> cycles_ahead >> cycles >> dma_cycles >> enable_delay;
	stream >> hdma_cur >> new_dma_cycles >> cgb_bgp_index >> cgb_obp_index;
	stream >> vram_bank >> entering_vblank >> cgb_mode >> cgb_bgp_autoinc;
	stream >> cgb_obp_autoinc >> hdma_active >> double_speed;
	stream >> unlocked_vram >> unlocked_oam;

	//recreate color pallette
	for (u32 i = 0; i < 64; ++i)
	{
		color_bgp[i / 8][(i % 8) / 2] = cgb_to_rgb(cgb_bgp[i]);
		color_obp[i / 8][(i % 8) / 2] = cgb_to_rgb(cgb_obp[i]);
	}
}
