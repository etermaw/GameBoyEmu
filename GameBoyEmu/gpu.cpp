#include "gpu.h"

enum IO_REGS {
	IO_LCD_CONTROL, IO_LCD_STATUS, IO_SY, IO_SX, IO_LY, IO_LYC,
	IO_DMA, IO_BGP, IO_OBP_0, IO_OBP_1, IO_WY, IO_WX7
};

enum LCD_CONTROL {
	LC_BG_ENABLED, LC_SPRITES_ENABLED, LC_SPRITES_SIZE, LC_BG_TMAP,
	LC_TILESET, LC_WINDOW, LC_WINDOW_TMAP, LC_POWER
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

u16 rgb_to_cgb(u32 rgb)
{
	u32 r = rgb & 0xFF;
	u32 g = (rgb & 0xFF00) >> 8;
	u32 b = (rgb & 0xFF0000) >> 16;

	r = ((r * 249 + 1014) >> 11) & 0x1F;
	g = ((g * 249 + 1014) >> 11) & 0x1F;
	b = ((b * 249 + 1014) >> 11) & 0x1F;

	return (b << 10) | (g << 5) | (r);
}

u32 cgb_to_rgb(u16 cgb)
{
	u32 r = cgb & 0x1F;
	u32 g = (cgb >> 5) & 0x1F;
	u32 b = (cgb >> 10) & 0x1F;

	r = ((r * 527 + 23) >> 6) & 0xFF;
	g = ((g * 527 + 23) >> 6) & 0xFF;
	b = ((b * 527 + 23) >> 6) & 0xFF;

	return 0xFF000000 | (b << 16) | (g << 8) | r;
}

u32 change_cgb_color(u32 old_color, u8 value, bool low)
{
	u16 decoded = rgb_to_cgb(old_color);

	if (low)
		decoded = (decoded & 0xFF00) | value;

	else
		decoded = (decoded && 0xFF00) | (value << 8);

	return cgb_to_rgb(decoded);
}

Gpu::Gpu(Interrupts& ints) : 
	interrupts(ints), regs(), cycles(0), dma_cycles(0), enable_delay(0), 
	entering_vblank(), cycles_ahead(0), vram_bank(0), cgb_mode(false)
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
}

void Gpu::vb_mode()
{
	regs[IO_LY]++;

	if (check_bit(regs[IO_LCD_STATUS], LS_LYC_LY) && regs[IO_LY] == regs[IO_LYC])
	{
		regs[IO_LCD_STATUS] = set_bit(regs[IO_LCD_STATUS], LS_CMP_SIG);
		interrupts.raise(INT_LCD);
	}

	if (regs[IO_LY] == 153)
	{
		regs[IO_LY] = 0;
		
		if (check_bit(regs[IO_LCD_STATUS], LS_LYC_LY) && regs[IO_LY] == regs[IO_LYC])
		{
			regs[IO_LCD_STATUS] = set_bit(regs[IO_LCD_STATUS], LS_CMP_SIG);
			interrupts.raise(INT_LCD);
		}

		regs[IO_LCD_STATUS] = (regs[IO_LCD_STATUS] & 0xFC) | 0x2; //go to mode 2

		if (check_bit(regs[IO_LCD_STATUS], LS_OAM))
			interrupts.raise(INT_LCD);
	}
}

void Gpu::hb_mode()
{
	regs[IO_LY]++;

	if (check_bit(regs[IO_LCD_STATUS], LS_LYC_LY) && regs[IO_LY] == regs[IO_LYC])
	{
		regs[IO_LCD_STATUS] = set_bit(regs[IO_LCD_STATUS], LS_CMP_SIG);
		interrupts.raise(INT_LCD);
	}

	if (regs[IO_LY] != 144)
		regs[IO_LCD_STATUS] = (regs[IO_LCD_STATUS] & 0xFC) | 0x2; //go to mode 2

	else
	{
		interrupts.raise(INT_VBLANK);

		if (check_bit(regs[IO_LCD_STATUS], LS_VBLANK))
			interrupts.raise(INT_LCD);

		regs[IO_LCD_STATUS] = (regs[IO_LCD_STATUS] & 0xFC) | 0x1; //go to mode 1
		entering_vblank = true;
	}
}

void Gpu::oam_mode()
{
	regs[IO_LCD_STATUS] = (regs[IO_LCD_STATUS] & 0xFC) | 0x3; //go to mode 3
}

void Gpu::transfer_mode()
{
	draw_line();

	if (check_bit(regs[IO_LCD_STATUS], LS_HBLANK))
		interrupts.raise(INT_LCD);

	regs[IO_LCD_STATUS] &= 0xFC; //go to mode 0

	if (hdma_active)
	{
		u16 src = (hdma_regs[0] << 8) | (hdma_regs[1] & 0xF0);
		u16 dst = ((hdma_regs[2] & 0x1F) << 8) | (hdma_regs[3] & 0xF0);
		u16 len = (hdma_regs[4] & 0x7F) + 1;
		u16 cur_pos = hdma_cur * 0x10;

		std::memcpy(&vram[vram_bank][dst + cur_pos], &ram_ptr[src + cur_pos], 0x10);

		if ((hdma_regs[4] & 0x7F) == 0)
		{
			hdma_active = false;
			hdma_regs[4] = 0xFF;
		}

		else
		{
			++hdma_cur;
			--hdma_regs[4];
		}

		step_ahead(8); //8 cycles passed, catch up
	}
}

void Gpu::step_ahead(u32 clock_cycles)
{
	if (dma_cycles > 0) //TODO: now it`s not affected by double speed, but it should!!!!!
		dma_cycles = std::max(0, dma_cycles - static_cast<int>(clock_cycles));

	if (!check_bit(regs[IO_LCD_CONTROL], LC_POWER))
		return;

	cycles += std::max(0, static_cast<i32>(clock_cycles) - enable_delay);
	enable_delay = std::max(0, enable_delay - static_cast<i32>(clock_cycles));

	static const u32 state_cycles[] = { 204, 456, 80, 172 };
	
	while (cycles >= state_cycles[regs[IO_LCD_STATUS] & 0x3])
	{
		cycles -= state_cycles[regs[IO_LCD_STATUS] & 0x3];

		switch (regs[IO_LCD_STATUS] & 0x3)
		{
			case 0:
				hb_mode();
				break;

			case 1:
				vb_mode();
				break;

			case 2:
				oam_mode();
				break;

			case 3:
				transfer_mode();
				break;
		}
	}
}

void Gpu::dma_copy(u8 adress)
{
	const u16 real_adress = adress * 0x100;

	//TODO: make it cycle accurant? (split it into per cycle read-write)
	if (real_adress >= 0x8000 && real_adress < 0xA000)
		std::memcpy(oam.get(), &vram[real_adress - 0x8000], sizeof(u8) * 0xA0);

	else if(real_adress >= 0xC000 && real_adress < 0xF000)
		std::memcpy(oam.get(), &ram_ptr[real_adress - 0xC000], sizeof(u8) * 0xA0);

	dma_cycles = 648;
	//when dma is launched, cpu can only access HRAM!
	//rumors says that OAM is blocked, but rest can be accessed (with bus conflicts)
	//but, cpu also can be interrupted
	//and when cpu is in double-speed mode, oam dma is 2x faster
}

void Gpu::draw_background_row()
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

	for (u32 i = 0; i < 160;) 
	{
		//it should work as singed/unsigned u8
		auto tile_num = (tile_nums[line_offset + (((sx + i) / 8) % 32)] + index_corrector) & 0xFF;

		u8 tile_low = tile_data[tile_num * 16 + tile_line * 2]; 
		u8 tile_high = tile_data[tile_num * 16 + tile_line * 2 + 1];

		for (u32 j = (sx + i) % 8; j < 8 && i < 160; ++j, ++i)
		{
			u32 id = 7 - j;
			u32 color_id = (check_bit(tile_high, id) << 1) | check_bit(tile_low, id);
			u32 shade_num = (regs[IO_BGP] >> (color_id * 2)) & 0x3;

			screen_buffer[buffer_offset + i] = get_dmg_color(shade_num);
		}
	}
}

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

void Gpu::draw_sprite_row()
{	
	const u8* tile_data = &vram[0][0]; //0x8000
	const bool sprite_size = check_bit(regs[IO_LCD_CONTROL], LC_SPRITES_SIZE);
	const u32 line = regs[IO_LY];
	const u32 height = sprite_size ? 16 : 8;
	const u32 line_offset = line * 160;
	const u32 bg_alpha_color = get_dmg_color(regs[IO_BGP] & 0x3);

	i32 count = 0;
	oam_entry to_draw[10];

	//in DMG sort sprites by (x,OAM id), then take first 10 which fits
	//in CBG, just take first 10 fitting line
	for (u32 i = 0; i < 40 && count < 10; ++i)
	{
		i32 y = oam[i*4] - 16;

		if (y <= line && ((y + height) > line))
		{
			to_draw[count].y = oam[i * 4];
			to_draw[count].x = oam[i * 4 + 1];
			to_draw[count].tile_num = oam[i * 4 + 2];
			to_draw[count++].atr = oam[i * 4 + 3];
		}
	}

	std::stable_sort(std::begin(to_draw), std::begin(to_draw) + count);
	
	for (i32 i = count - 1; i >= 0; --i)
	{
		i32 sx = to_draw[i].x - 8;
		i32 sy = to_draw[i].y - 16;
		u32 tile_num = to_draw[i].tile_num;
		u32 atr = to_draw[i].atr;

		u32 tile_line = line - sy;
		const u32 palette_num = IO_OBP_0 + check_bit(atr, 4); //0 - OBP[0], 1 - OBP[1]

		if (check_bit(atr, 6)) //Y flip
			tile_line = height - tile_line - 1;
				
		if (sprite_size)
			tile_num = tile_line < 8 ? (tile_num & 0xFE) : (tile_num | 0x1);
		
		u8 tile_low = tile_data[tile_num * 16 + tile_line * 2];
		u8 tile_high = tile_data[tile_num * 16 + tile_line * 2 + 1];

		if (check_bit(atr, 5)) //X flip
		{
			tile_low = flip_bits(tile_low);
			tile_high = flip_bits(tile_high);
		}

		const u32 begin = std::max(0, sx);
		const u32 end = std::min(sx + 8, 160);

		//what if BG+window is turned off? (for now we assume that it`s only enabled)
		if (check_bit(atr, 7)) //BG has priority
		{
			for (u32 j = begin; j < end; ++j) 
			{
				u32 id = end - j - 1;
				u32 color_id = (check_bit(tile_high, id) << 1) | check_bit(tile_low, id);
				u32 color = get_dmg_color((regs[palette_num] >> (color_id * 2)) & 0x3);

				if (color_id != 0 && screen_buffer[line_offset + j] == bg_alpha_color)
					screen_buffer[line_offset + j] = color;
			}
		}

		else //sprite has priority
		{
			for (u32 j = begin; j < end; ++j)
			{
				u32 id = end - j - 1;
				u32 color_id = (check_bit(tile_high, id) << 1) | check_bit(tile_low, id);
				u32 color = get_dmg_color((regs[palette_num] >> (color_id * 2)) & 0x3);

				if (color_id != 0)
					screen_buffer[line_offset + j] = color;
			}
		}
	}
}

void Gpu::draw_window_row() 
{
	if (regs[IO_LY] < regs[IO_WY] || regs[IO_WX7] > 166) 
		return;

	const u32 line = regs[IO_LY];
	const u32 wy = regs[IO_WY];
	const i32 wx = regs[IO_WX7] - 7;
	
	const u32 offset = check_bit(regs[IO_LCD_CONTROL], LC_WINDOW_TMAP) ? 0x1C00 : 0x1800; //0x9C00,0x9800
	const u32 data_offset = check_bit(regs[IO_LCD_CONTROL], LC_TILESET) ? 0 : 0x800; //0x8000,0x8800
	const u32 index_corrector = check_bit(regs[IO_LCD_CONTROL], LC_TILESET) ? 0 : 128;
	const u32 buffer_offset = line * 160;

	const u8* tile_nums = &vram[0][offset];
	const u8* tile_data = &vram[0][data_offset]; //data_offset == index_corrector * 16

	const u32 window_line = line - wy;
	const u32 tile_line = window_line % 8;
	const u32 line_off = (window_line / 8) * 32;

	const u32 start_offset = -std::min(wx, 0);

	for (u32 i = std::max(0, wx); i < 160;) 
	{
		u32 tile_num = (tile_nums[line_off + (i + start_offset) / 8] + index_corrector) & 0xFF;
		u8 tile_low = tile_data[tile_num * 16 + tile_line * 2];
		u8 tile_high = tile_data[tile_num * 16 + tile_line * 2 + 1];

		for (u32 j = (start_offset + i) % 8; j < 8 && i < 160; ++j, ++i)
		{
			u32 id = 7 - j;
			u32 color_id = (check_bit(tile_high, id) << 1) | check_bit(tile_low, id);
			u32 shade_num = (regs[IO_BGP] >> (color_id * 2)) & 0x3;

			screen_buffer[buffer_offset + i] = get_dmg_color(shade_num);
		}
	}
}

void Gpu::draw_background_row_cgb()
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

	const u32 line_offset = (((line + sy) / 8) % 32) * 32;
	u32 tile_line = (line + sy) % 8;

	for (u32 i = 0; i < 160;)
	{
		//it should work as singed/unsigned u8
		auto tile_num = (tile_nums[line_offset + (((sx + i) / 8) % 32)] + index_corrector) & 0xFF;
		auto tile_atr = tile_atrs[line_offset + (((sx + i) / 8) % 32)];

		u8 palette_num = tile_atr & 0x7;
		u8 data_bank = check_bit(tile_atr, 3);
		bool priority = check_bit(tile_atr, 7);

		if (check_bit(tile_atr, 6)) //Y flip
			tile_line = 8 - tile_line;

		u8 tile_low = tile_data[data_bank][tile_num * 16 + tile_line * 2];
		u8 tile_high = tile_data[data_bank][tile_num * 16 + tile_line * 2 + 1];

		if (check_bit(tile_atr, 5)) //X flip
		{
			tile_low = flip_bits(tile_low);
			tile_high = flip_bits(tile_high);
		}

		for (u32 j = (sx + i) % 8; j < 8 && i < 160; ++j, ++i)
		{
			u32 id = 7 - j;
			u32 color_id = (check_bit(tile_high, id) << 1) | check_bit(tile_low, id);

			screen_buffer[buffer_offset + i] = color_bgp[palette_num][color_id];
			//priority_buffer[line_offset + i] = priority;
		}
	}
}

void Gpu::draw_window_row_cgb()
{
	if (regs[IO_LY] < regs[IO_WY] || regs[IO_WX7] > 166)
		return;

	const u32 line = regs[IO_LY];
	const u32 wy = regs[IO_WY];
	const i32 wx = regs[IO_WX7] - 7;

	const u32 offset = check_bit(regs[IO_LCD_CONTROL], LC_WINDOW_TMAP) ? 0x1C00 : 0x1800; //0x9C00,0x9800
	const u32 data_offset = check_bit(regs[IO_LCD_CONTROL], LC_TILESET) ? 0 : 0x800; //0x8000,0x8800
	const u32 index_corrector = check_bit(regs[IO_LCD_CONTROL], LC_TILESET) ? 0 : 128;
	const u32 buffer_offset = line * 160;

	const u8* tile_nums = &vram[0][offset];
	const u8* tile_atrs = &vram[1][offset];
	const u8* tile_data[2] = { &vram[0][data_offset], &vram[1][data_offset] };
	
	const u32 window_line = line - wy;
	u32 tile_line = window_line % 8;
	const u32 line_off = (window_line / 8) * 32;

	const u32 start_offset = -std::min(wx, 0);

	for (u32 i = std::max(0, wx); i < 160;)
	{
		u32 tile_num = (tile_nums[line_off + (i + start_offset) / 8] + index_corrector) & 0xFF;
		u8 tile_atr = tile_atrs[line_off + (i + start_offset) / 8];

		u8 palette_num = tile_atr & 0x7;
		u8 data_bank = check_bit(tile_atr, 3);
		bool priority = check_bit(tile_atr, 7);

		if (check_bit(tile_atr, 6)) //Y flip
			tile_line = 8 - tile_line;

		u8 tile_low = tile_data[data_bank][tile_num * 16 + tile_line * 2];
		u8 tile_high = tile_data[data_bank][tile_num * 16 + tile_line * 2 + 1];

		if (check_bit(tile_atr, 5)) //X flip
		{
			tile_low = flip_bits(tile_low);
			tile_high = flip_bits(tile_high);
		}

		for (u32 j = (start_offset + i) % 8; j < 8 && i < 160; ++j, ++i)
		{
			u32 id = 7 - j;
			u32 color_id = (check_bit(tile_high, id) << 1) | check_bit(tile_low, id);

			screen_buffer[buffer_offset + i] = color_bgp[palette_num][color_id];
			//priority_buffer[line_offset + i] = priority;
		}
	}
}

//priorities for GBC:  BG0 < OBJL < BGL < OBJH < BGH
void Gpu::draw_sprite_row_cgb()
{
	const u8* tile_data[2] = { &vram[0][0], &vram[1][0] }; //0x8000
	const bool sprite_size = check_bit(regs[IO_LCD_CONTROL], LC_SPRITES_SIZE);
	const u32 line = regs[IO_LY];
	const u32 height = sprite_size ? 16 : 8;
	const u32 line_offset = line * 160;
	const u32 bg_alpha_color = get_dmg_color(regs[IO_BGP] & 0x3);

	i32 count = 0;
	oam_entry to_draw[10]; //if we don`t sort, we don`t need to init with 0xFF

	for (u32 i = 0; i < 40 && count < 10; ++i)
	{
		i32 y = oam[i * 4] - 16;

		if (y <= line && ((y + height) > line))
		{
			to_draw[count].y = oam[i * 4];
			to_draw[count].x = oam[i * 4 + 1];
			to_draw[count].tile_num = oam[i * 4 + 2];
			to_draw[count++].atr = oam[i * 4 + 3];
		}
	}

	for (i32 i = count - 1; i >= 0; --i)
	{
		i32 sx = to_draw[i].x - 8;
		i32 sy = to_draw[i].y - 16;
		u32 tile_num = to_draw[i].tile_num;
		u32 atr = to_draw[i].atr;

		u32 tile_line = line - sy;
		const u32 palette_num = atr & 0x7;
		const u32 bank_num = check_bit(atr, 3);

		if (check_bit(atr, 6)) //Y flip
			tile_line = height - tile_line - 1;

		if (sprite_size)
			tile_num = tile_line < 8 ? (tile_num & 0xFE) : (tile_num | 0x1);

		u8 tile_low = tile_data[bank_num][tile_num * 16 + tile_line * 2];
		u8 tile_high = tile_data[bank_num][tile_num * 16 + tile_line * 2 + 1];

		if (check_bit(atr, 5)) //X flip
		{
			tile_low = flip_bits(tile_low);
			tile_high = flip_bits(tile_high);
		}

		const u32 begin = std::max(0, sx);
		const u32 end = std::min(sx + 8, 160);

		//if bit 7 == 1, then sprite will cover ONLY BG color 0
		//else sprite will cover ONLY if priority_buffer != 1

		for (u32 j = begin; j < end; ++j)
		{
			u32 id = end - j - 1;
			u32 color_id = (check_bit(tile_high, id) << 1) | check_bit(tile_low, id);
			u32 color = color_obp[palette_num][color_id];

			if (check_bit(atr, 7))
			{
				if (color_id != 0 && screen_buffer[line_offset + j] == bg_alpha_color)
					screen_buffer[line_offset + j] = color;
			}

			else
			{
				if (color_id != 0 /*&& !priority_buffer[line_offset + j]*/)
					screen_buffer[line_offset + j] = color;
			}
		}
	}
}

void Gpu::draw_line()
{
	if (check_bit(regs[IO_LCD_CONTROL], LC_POWER))
	{
		if (check_bit(regs[IO_LCD_CONTROL], LC_BG_ENABLED))
			draw_background_row();

		if (check_bit(regs[IO_LCD_CONTROL], LC_WINDOW))
			draw_window_row();

		if (check_bit(regs[IO_LCD_CONTROL], LC_SPRITES_ENABLED))
			draw_sprite_row();
	}
}

void Gpu::turn_off_lcd()
{
	std::memset(screen_buffer.get(), 0xFF, sizeof(u32) * 160 * 140);
	regs[IO_LY] = 0;
	regs[IO_LYC] = 0;
	regs[IO_LCD_STATUS] &= 0xFC; //mode 0
	cycles = 0;

	entering_vblank = true;
}

void Gpu::turn_on_lcd()
{
	if (!check_bit(regs[IO_LCD_CONTROL], LC_POWER)) //useless conditional?
		enable_delay = 244;
}

void Gpu::launch_gdma()
{
	u16 src = (hdma_regs[0] << 8) | (hdma_regs[1] & 0xF0);
	u16 dst = ((hdma_regs[2] & 0x1F) << 8) | (hdma_regs[3] & 0xF0);
	u16 len = ((hdma_regs[4] & 0x7F) + 1) * 0x10;

	std::memcpy(&vram[vram_bank][dst], &ram_ptr[src], len); //TODO: if we copy during mode 3, vram won`t change!

	step_ahead(len / 2); // len/2 cycles passed, so we need to catch up
	hdma_regs[4] = 0xFF;
}

u8 Gpu::read_byte(u16 adress, u32 cycles_passed)
{
	cycles_passed -= cycles_ahead;

	if (cycles_passed > 0)
	{
		step_ahead(cycles_passed);
		cycles_ahead += cycles_passed;
	}

	//if gpu is in mode 3 ignore read (return 0xFF instead)
	if (adress >= 0x8000 && adress < 0xA000)
		return ((regs[IO_LCD_STATUS] & 0x3) != 0x3) ? vram[vram_bank][adress - 0x8000] : 0xFF;

	//if gpu is in mode 2,3 or during oam dma, ignore read
	else if (adress >= 0xFE00 && adress < 0xFEA0)
		return ((regs[IO_LCD_STATUS] & 0x3) < 0x2 && dma_cycles <= 0) ? oam[adress - 0xFE00] : 0xFF;

	else if (adress >= 0xFF40 && adress <= 0xFF4B)
		return regs[adress - 0xFF40];

	else if (cgb_mode && adress == 0xFF4F)
		return vram_bank & 0xFE;

	else if (cgb_mode && adress >= 0xFF51 && adress < 0xFF55)
		return hdma_regs[adress - 0xFF51]; //TODO: mask out useless bits?

	else if (cgb_mode && adress == 0xFF55)
		return change_bit(hdma_regs[4], !hdma_active, 7);

	else if (cgb_mode && adress == 0xFF68)
		return change_bit(cgb_bgp_index, cgb_bgp_autoinc, 7);

	else if (cgb_mode && adress == 0xFF69)
	{
		if ((regs[IO_LCD_STATUS] & 0x3) != 0x3)
		{
			u32 encoded = color_bgp[cgb_bgp_index / 8][(cgb_bgp_index % 8) / 2];
			u16 decoded = rgb_to_cgb(encoded);

			return (decoded >> (cgb_bgp_index % 2)) & 0xFF;
		}

		else
			return 0xFF;
	}

	else if (cgb_mode && adress == 0xFF6A)
		return change_bit(cgb_obp_index, cgb_obp_autoinc, 7);

	else if (cgb_mode && adress == 0xFF6B)
	{
		if ((regs[IO_LCD_STATUS] & 0x3) != 0x3)
		{
			u32 encoded = color_obp[cgb_obp_index / 8][(cgb_obp_index % 8) / 2];
			u16 decoded = rgb_to_cgb(encoded);

			return (decoded >> (cgb_obp_index % 2)) & 0xFF;
		}

		else
			return 0xFF;
	}
}

void Gpu::write_byte(u16 adress, u8 value, u32 cycles_passed)
{
	cycles_passed -= cycles_ahead;

	if (cycles_passed > 0)
	{
		step_ahead(cycles_passed);
		cycles_ahead += cycles_passed;
	}

	//if gpu is in mode 3 ignore write
	if (adress >= 0x8000 && adress < 0xA000 && ((regs[IO_LCD_STATUS] & 0x3) != 0x3))
		vram[vram_bank][adress - 0x8000] = value;

	//if gpu is in mode 2,3 or during oam dma, ignore write
	else if (adress >= 0xFE00 && adress < 0xFEA0 && ((regs[IO_LCD_STATUS] & 0x3) < 0x2) && dma_cycles <= 0)
		oam[adress - 0xFE00] = value;

	else if (adress >= 0xFF40 && adress <= 0xFF4B)
	{
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
			regs[IO_LY] = 0;

		else if (adress == 0xFF46)
			dma_copy(value);

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

	else if (cgb_mode && adress == 0xFF69 && ((regs[IO_LCD_STATUS] & 0x3) != 0x3))
	{
		//convert rgb15 into rgb32
		auto color = color_bgp[cgb_bgp_index / 8][(cgb_bgp_index % 8) / 2];
		color = change_cgb_color(color, value, cgb_bgp_index % 2);
		color_bgp[cgb_bgp_index / 8][(cgb_bgp_index % 8) / 2] = color;

		if (cgb_bgp_autoinc)
			cgb_bgp_index = (cgb_bgp_index + 1) & 0x3F;
	}

	else if (cgb_mode && adress == 0xFF6A)
	{
		cgb_obp_index = value & 0x3F;
		cgb_obp_autoinc = check_bit(value, 7);
	}

	else if (cgb_mode && adress == 0xFF6B && ((regs[IO_LCD_STATUS] & 0x3) != 0x3))
	{
		//convert rgb15 into rgb32
		auto color = color_obp[cgb_obp_index / 8][cgb_obp_index % 4];
		color = change_cgb_color(color, value, cgb_obp_index % 2);
		color_obp[cgb_obp_index / 8][cgb_obp_index % 4] = color;

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

void Gpu::step(u32 clock_cycles)
{
	step_ahead(clock_cycles - cycles_ahead);
	cycles_ahead = 0;
}