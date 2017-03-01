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

Gpu::Gpu() : screen_buffer(std::make_unique<u32[]>(144 * 160)), regs(), 
			 cycles(0), dma_cycles(0), enable_delay(0), entering_vblank()
{
	regs[IO_LCD_CONTROL] = 0x91;
	regs[IO_BGP] = 0xFC;
	regs[IO_OBP_0] = 0xFF;
	regs[IO_OBP_1] = 0xFF;

	vram = std::make_unique<u8[]>(0x2000);
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
	//if gpu is in mode 3 ignore read
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
			regs[IO_LY] = 0;

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
	const u8* src = memory_callback(real_adress);

	std::memcpy(oam, src, sizeof(u8) * 0xA0);

	dma_cycles = 672; //~160 us, should be correct
	//however, other spec says that it takes 160 * 4 + 4 cycles (644)
	//aaaand when dma is launched, cpu can only access HRAM!
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
	const u8* tile_data = &vram[data_offset];

	const u32 line_offset = (((line + sy) / 8) % 32) * 32;
	const u32 tile_line = (line + sy) % 8;

	for (u32 i = 0; i < 160;) 
	{
		auto tile_num = tile_nums[line_offset + (((sx + i) / 8) % 32)] + index_corrector;
		tile_num &= 0xFF; //it should work as singed/unsigned u8

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

void Gpu::draw_sprite_row()
{	
}

void Gpu::draw_window_row() 
{
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
	regs[IO_LCD_STATUS] &= 0xFD;
	cycles = 0;
}

void Gpu::turn_on_lcd()
{
	if (!check_bit(regs[IO_LCD_CONTROL], LC_POWER))
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
