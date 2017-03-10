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

Gpu::Gpu() : regs(), cycles(0), dma_cycles(0), enable_delay(0), entering_vblank()
{
	regs[IO_LCD_CONTROL] = 0x91;
	regs[IO_BGP] = 0xFC;
	regs[IO_OBP_0] = 0xFF;
	regs[IO_OBP_1] = 0xFF;

	vram = std::make_unique<u8[]>(0x2000);
	screen_buffer = std::make_unique<u32[]>(144 * 160);

	std::memset(screen_buffer.get(), 0xFF, sizeof(u32) * 144 * 160);
	std::memset(oam, 0xFF, sizeof(u8) * 0xA0);
}

void Gpu::vb_mode(Interrupts& interrupts)
{
	if (cycles >= 456)
	{
		cycles -= 456;
		regs[IO_LY]++;

		if (regs[IO_LY] == 153)
		{
			if (check_bit(regs[IO_LCD_STATUS], LS_OAM))
				interrupts.raise(INT_LCD);

			regs[IO_LY] = 0;
			regs[IO_LCD_STATUS] = (regs[IO_LCD_STATUS] & 0xFC) | 0x2; //go to mode 2
		}
	}
}

void Gpu::hb_mode(Interrupts& interrupts)
{
	if (cycles >= 204)
	{
		cycles -= 204;
		regs[IO_LY]++;

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
}

void Gpu::oam_mode(Interrupts& interrupts)
{
	if (cycles >= 80)
	{
		cycles -= 80;
		regs[IO_LCD_STATUS] = (regs[IO_LCD_STATUS] & 0xFC) | 0x3; //go to mode 3
	}
}

void Gpu::transfer_mode(Interrupts& interrupts)
{
	if (cycles >= 172)
	{
		draw_line();

		if (check_bit(regs[IO_LCD_STATUS], LS_HBLANK))
			interrupts.raise(INT_LCD);

		cycles -= 172;
		regs[IO_LCD_STATUS] &= 0xFC; //go to mode 0
	}
}

u8 Gpu::read_byte(u16 adress)
{
	//if gpu is in mode 3 ignore read (return 0xFF instead)
	if (adress >= 0x8000 && adress < 0xA000)
		return ((regs[IO_LCD_STATUS] & 0x3) != 0x3) ? vram[adress - 0x8000] : 0xFF;

	//if gpu is in mode 2 or 3, ignore read
	else if (adress >= 0xFE00 && adress < 0xFEA0)
		return ((regs[IO_LCD_STATUS] & 0x3) < 0x2) ? oam[adress - 0xFE00] : 0xFF;

	else if (adress >= 0xFF40 && adress <= 0xFF4B)
		return regs[adress - 0xFF40];
}

void Gpu::write_byte(u16 adress, u8 value)
{
	//if gpu is in mode 3 ignore write
	if (adress >= 0x8000 && adress < 0xA000 && ((regs[IO_LCD_STATUS] & 0x3) != 0x3))
		vram[adress - 0x8000] = value;

	//if gpu is in mode 2 or 3, ignore write
	else if (adress >= 0xFE00 && adress < 0xFEA0 && ((regs[IO_LCD_STATUS] & 0x3) < 0x2))
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
			regs[IO_LY] = 0; //TODO: check LY==LYC

		else if (adress == 0xFF46)
			dma_copy(value);

		else
			regs[adress - 0xFF40] = value;
	}

	else
		int a = 0;
}

bool Gpu::is_entering_vblank()
{
	bool tmp = entering_vblank;
	entering_vblank = false;

	return tmp;
}

void Gpu::dma_copy(u8 adress)
{
	const u16 real_adress = adress * 0x100;

	//TODO: make it cycle accurant? (split it into per cycle read-write)
	if (real_adress >= 0x8000 && real_adress < 0xA000)
		std::memcpy(oam, &vram[real_adress - 0x8000], sizeof(u8) * 0xA0);

	else if(real_adress >= 0xC000 && real_adress < 0xF000)
		std::memcpy(oam, &ram_ptr[real_adress - 0xC000], sizeof(u8) * 0xA0);

	dma_cycles = 672; //~160 us, should be correct
	//however, other spec says that it takes 160 * 4 + 4 cycles (644)
	//aaaand when dma is launched, cpu can only access HRAM!
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
	
	const u8* tile_nums = &vram[offset]; 
	const u8* tile_data = &vram[data_offset]; //index_corrector * 16 == data_offset

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

// priorities for GBC:  BG0 < OBJL < BGL < OBJH < BGH
void Gpu::draw_sprite_row()
{	
	const u8* tile_data = &vram[0]; //0x8000
	const bool sprite_size = check_bit(regs[IO_LCD_CONTROL], LC_SPRITES_SIZE);
	const u32 line = regs[IO_LY];
	const u32 height = sprite_size ? 16 : 8;
	const u32 line_offset = line * 160;
	const u32 bg_alpha_color = get_dmg_color(regs[IO_BGP] & 0x3);

	i32 count = 0;
	oam_entry to_draw[10];

	std::memset(to_draw, 0xFF, sizeof(oam_entry) * 10);

	//in DMG sort sprites by (x,OAM id), then take first 10 which fits
	//in CBG, just take first 10 fitting line
	for (u32 i = 0; i < 40 && count < 10; ++i)
	{
		i32 y = oam[i*4] - 16; //full sort if oam-dma launched, if single x is modded, just resort it

		if (y <= line && ((y + height) > line)) //should be ok now
		{
			to_draw[count].y = oam[i * 4];
			to_draw[count].x = oam[i * 4 + 1];
			to_draw[count].tile_num = oam[i * 4 + 2];
			to_draw[count++].atr = oam[i * 4 + 3];
		}
	}

	std::stable_sort(std::begin(to_draw), std::end(to_draw));
	
	for (i32 i = count - 1; i >= 0; --i)
	{
		i32 sx = to_draw[i].x - 8;
		i32 sy = to_draw[i].y - 16;
		u32 tile_num = to_draw[i].tile_num;
		u32 atr = to_draw[i].atr;

		u32 tile_line = line - sy;
		const u32 palette_num = IO_OBP_0 + check_bit(atr, 4); //0 - OBP[0], 1 - OBP[1]

		if (check_bit(atr, 6)) //Y flip
			tile_line = height - tile_line - 1; //should be correct now
				
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
				u32 id = end - j - 1; //should be ok
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
				u32 id = end - j - 1; //should be ok
				u32 color_id = (check_bit(tile_high, id) << 1) | check_bit(tile_low, id);
				u32 color = get_dmg_color((regs[palette_num] >> (color_id * 2)) & 0x3);

				if (color_id != 0)
					screen_buffer[line_offset + j] = color;
			}
		}
	}

	//select 10 sprites (asc order by x, then by number in oam)
	//draw them starting from last one (DMG mode, in CGB priority is always assigned by OAM index)
	//check priority bit, BGP[0] will be always covered by OBJ pixel!!!!
}

void Gpu::draw_window_row() 
{
	if (regs[IO_LY] < regs[IO_WY] || regs[IO_WX7] > 166) 
		return;

	const u32 line = regs[IO_LY];
	const u32 wy = regs[IO_WY];
	const i32 wx = regs[IO_WX7] - 7;
	
	const u32 offset = check_bit(regs[IO_LCD_CONTROL], LC_BG_TMAP) ? 0x1C00 : 0x1800; //0x9C00,0x9800
	const u32 data_offset = check_bit(regs[IO_LCD_CONTROL], LC_TILESET) ? 0 : 0x800; //0x8000,0x8800
	const u32 index_corrector = check_bit(regs[IO_LCD_CONTROL], LC_TILESET) ? 0 : 128;
	const u32 buffer_offset = line * 160;

	const u8* tile_nums = &vram[offset];
	const u8* tile_data = &vram[data_offset]; //data_offset == index_corrector * 16

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
	regs[IO_LCD_STATUS] &= 0xFD; //mode 1
	cycles = 0;

	entering_vblank = true;
}

void Gpu::turn_on_lcd()
{
	if (!check_bit(regs[IO_LCD_CONTROL], LC_POWER)) //useless conditional?
		enable_delay = 244;
}

u32 Gpu::step(u32 clock_cycles, Interrupts& interrupts)
{
	if (dma_cycles > 0)
		dma_cycles = std::max(0, dma_cycles - static_cast<int>(clock_cycles));

	if (!check_bit(regs[IO_LCD_CONTROL], LC_POWER))
		return 0;

	else if (enable_delay > 0)
	{
		enable_delay = std::max(0, enable_delay - static_cast<int>(clock_cycles));
		return 0;
	}

	cycles += clock_cycles;

	switch (regs[IO_LCD_STATUS] & 0x3)
	{
		case 0: 
			hb_mode(interrupts);
			break;

		case 1:
			vb_mode(interrupts);
			break;

		case 2:
			oam_mode(interrupts);
			break;

		case 3: 
			transfer_mode(interrupts);
			break;
	}

	if (check_bit(regs[IO_LCD_STATUS], LS_LYC_LY) && regs[IO_LY] == regs[IO_LYC])
	{
		regs[IO_LCD_STATUS] = set_bit(regs[IO_LCD_STATUS], LS_CMP_SIG);
		interrupts.raise(INT_LCD);
	}

	return 0;
}
