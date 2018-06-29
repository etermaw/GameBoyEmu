#include "mbc.h"

void MBCBase::switch_rom_bank(u32 new_bank_rom)
{
	//cartrige code sets this to 2 << N, so any other values are errors
	assert(is_pow_of_2(max_rom_banks) && max_rom_banks >= 2);

	rom_bank = new_bank_rom & (max_rom_banks - 1);
}

void MBCBase::switch_bank_ram(u32 new_ram_bank)
{
	//catch all errors, value 0 should be impossible (check constructor)
	assert(is_pow_of_2(max_banks_ram) && max_banks_ram >= 1);

	ram_bank = new_ram_bank & (max_banks_ram - 1);
}

u8 NoMBC::read_byte(u16 adress, u32 cycles_passed)
{
	UNUSED(cycles_passed);

	if (adress < 0x8000)
		return rom[adress];

	else if (adress >= 0xA000 && adress < 0xC000)
	{
		const u32 real_address = adress - 0xA000;

		if (ram_enabled && real_address < ram_size)
			return ram[real_address];

		else
			return 0xFF;
	}

	else
		return 0xFF;
}

void NoMBC::write_byte(u16 adress, u8 value, u32 cycles_passed)
{
	UNUSED(cycles_passed);
	
	if (adress < 0x2000)
		ram_enabled = ((value & 0x0F) == 0x0A) && (ram != nullptr);

	else if (adress >= 0xA000 && adress < 0xC000 && ram_enabled)
	{
		const u32 real_address = adress - 0xA000;

		if (ram_enabled && real_address < ram_size)
			ram[real_address] = value;
	}

	//else ignore
}

const u8* NoMBC::get_dma_ptr(u16 adress)
{
	if (adress < 0x8000)
		return &rom[adress];

	else if (adress >= 0xA000 && adress < 0xC000 && ram_enabled) //TODO: handling out of range requests
		return &ram[adress - 0xA000];

	else
		return nullptr;
}

u32 MBC1::calculate_offset(u32 high_rom_num)
{
	const u32 lanes = (max_rom_banks == 64 ? 0x1 : (max_rom_banks == 128 ? 0x3 : 0));

	return ((high_rom_num & lanes) << 5) * 0x4000;
}

u8 MBC1::read_byte(u16 adress, u32 cycles_passed)
{
	UNUSED(cycles_passed);
	
	if (adress < 0x4000)
		return rom[adress + high_offset];

	else if (adress < 0x8000)
		return rom[adress - 0x4000 + (rom_bank * 0x4000)];

	else if (adress >= 0xA000 && adress < 0xC000)
	{
		const u32 real_address = adress - 0xA000 + (ram_bank * 0x2000);

		if (ram_enabled && real_address < ram_size)
			return ram[real_address];

		else
			return 0xFF;
	}

	else
		return 0xFF;
}

void MBC1::write_byte(u16 adress, u8 value, u32 cycles_passed)
{
	UNUSED(cycles_passed);
	
	if (adress < 0x2000)
		ram_enabled = ((value & 0x0F) == 0x0A) && (ram != nullptr);

	else if (adress < 0x4000)
	{
		rom_num_low = std::max(1, value & 0x1F);
		switch_rom_bank(((ram_mode ? 0 : rom_num_high) << 5) | rom_num_low);
	}

	else if (adress < 0x6000)
	{
		rom_num_high = value & 0x3;
		switch_rom_bank((rom_num_high << 5) | rom_num_low);

		if (ram_mode)
		{
			switch_bank_ram(rom_num_high);
			high_offset = calculate_offset(rom_num_high);
		}

	}

	else if (adress < 0x8000)
	{
		ram_mode = check_bit(value, 0);

		if (ram_mode)
			high_offset = calculate_offset(rom_num_high);

		else
		{
			switch_bank_ram(0);
			high_offset = 0;
		}
	}

	else if (adress >= 0xA000 && adress < 0xC000)
	{
		const u32 real_address = adress - 0xA000 + (ram_bank * 0x2000);

		if (ram_enabled && real_address < ram_size)
			ram[real_address] = value;
	}
}

const u8* MBC1::get_dma_ptr(u16 adress)
{
	if (adress < 0x4000)
		return &rom[adress];

	else if (adress < 0x8000)
		return &rom[adress - 0x4000 + (rom_bank * 0x4000)];

	else if (adress >= 0xA000 && adress < 0xC000 && ram_enabled)
		return &ram[adress - 0xA000 + (ram_bank * 0x2000)];

	else
		return nullptr;
}

u8 MBC2::read_byte(u16 adress, u32 cycles_passed)
{
	UNUSED(cycles_passed);
	
	if (adress < 0x4000)
		return rom[adress];

	else if (adress < 0x8000)
		return rom[adress - 0x4000 + (rom_bank * 0x4000)];

	else if (adress >= 0xA000 && adress < 0xA200)
		return (ram_enabled ? ram[adress - 0xA000] : 0xFF);

	else
		return 0xFF;
}

void MBC2::write_byte(u16 adress, u8 value, u32 cycles_passed)
{
	UNUSED(cycles_passed);
	
	if (adress < 0x2000 && ((adress & 0x0100) == 0x0000))
		ram_enabled = ((value & 0x0F) == 0x0A);

	else if (adress < 0x4000 && ((adress & 0x0100) == 0x0100))
		switch_rom_bank(value & 0x0F);

	else if (adress >= 0xA000 && adress < 0xA200 && ram_enabled)
		ram[adress - 0xA000] = value & 0xF;
}

const u8* MBC2::get_dma_ptr(u16 adress)
{
	if (adress < 0x4000)
		return &rom[adress];

	else if (adress < 0x8000)
		return &rom[adress - 0x4000 + (rom_bank * 0x4000)];

	else if (adress >= 0xA000 && adress < 0xA200 && ram_enabled)
		return &ram[adress - 0xA000];

	else
		return nullptr;
}

void MBC3::latch_rtc()
{
	if (!check_bit(rtc[4], 6)) //if timer is enabled, update time
		update_time();

	std::memcpy(latched_rtc, rtc, sizeof(u8) * 5);
}

void MBC3::update_time()
{
	auto cur = std::chrono::system_clock::now();
	auto delta = std::chrono::duration_cast<std::chrono::seconds>(cur - start_time);

	auto new_seconds = rtc[0] + delta.count();
	auto new_minutes = rtc[1] + new_seconds / 60;
	auto new_hours = rtc[2] + new_minutes / 60;
	auto new_days = (((rtc[4] & 1) << 8) | rtc[3]) + (new_hours / 24);

	rtc[0] = new_seconds % 60;
	rtc[1] = new_minutes % 60;
	rtc[2] = new_hours % 24;
	rtc[3] = new_days % 512;
	rtc[4] = change_bit(rtc[4], (new_days % 512) > 255, 0);
	rtc[4] = change_bit(rtc[4], new_days > 511, 7);
	start_time = cur;
}

u8 MBC3::read_byte(u16 adress, u32 cycles_passed)
{
	UNUSED(cycles_passed);
	
	if (adress < 0x4000)
		return rom[adress];

	else if (adress < 0x8000)
		return rom[adress - 0x4000 + (rom_bank * 0x4000)];

	else if (adress >= 0xA000 && adress < 0xC000 && ram_enabled)
	{
		if (reg_used)
			return latched_rtc[selected_time_reg];

		else
		{
			const u32 real_address = adress - 0xA000 + (ram_bank * 0x2000);

			if (real_address < ram_size)
				return ram[real_address];

			else
				return 0xFF;
		}
	}

	else
		return 0xFF;
}

void MBC3::write_byte(u16 adress, u8 value, u32 cycles_passed)
{
	UNUSED(cycles_passed);
	
	if (adress < 0x2000)
		ram_enabled = ((value & 0x0F) == 0x0A) && (ram != nullptr || rtc != nullptr); //ram_enabled affects ram AND timer

	else if (adress < 0x4000)
		switch_rom_bank(std::max(1, value & 0x7F));
	
	else if (adress < 0x6000)
	{
		if (value < 0x04) 
		{
			reg_used = false;
			switch_bank_ram(value);
		}

		else if (rtc != nullptr && value >= 0x08 && value <= 0x0C) //TODO: keep an eye on it
		{
			reg_used = true;
			selected_time_reg = std::max(value - 0x08, 5);
		}
	}

	else if (adress < 0x8000 && rtc != nullptr)
	{
		if (last_write == 0x00 && value == 0x01)
			latch_rtc();

		last_write = value;
	}

	else if (adress >= 0xA000 && adress < 0xC000 && ram_enabled)
	{
		if (reg_used)
			rtc[selected_time_reg] = value;

		else
		{
			const u32 real_address = adress - 0xA000 + (ram_bank * 0x2000);

			if (real_address < ram_size)
				ram[real_address] = value;
		}
	}
}

const u8* MBC3::get_dma_ptr(u16 adress)
{
	if (adress < 0x4000)
		return &rom[adress];

	else if (adress < 0x8000)
		return &rom[adress - 0x4000 + (rom_bank * 0x4000)];

	else if (adress >= 0xA000 && adress < 0xC000 && ram_enabled && !reg_used) //TODO:what if someone launch dma with rtc enabled?
		return &ram[adress - 0xA000 + (ram_bank * 0x2000)];

	else
		return nullptr;
}

u8 MBC5::read_byte(u16 adress, u32 cycles_passed)
{
	UNUSED(cycles_passed);
	
	if (adress < 0x4000)
		return rom[adress];

	else if (adress < 0x8000)
		return rom[adress - 0x4000 + (rom_bank * 0x4000)];

	else if (adress >= 0xA000 && adress < 0xC000)
	{
		const u32 real_address = adress - 0xA000 + (ram_bank * 0x2000);

		if (ram_enabled && real_address < ram_size)
			return ram[real_address];

		else
			return 0xFF;
	}

	else
		return 0xFF;
}

void MBC5::write_byte(u16 adress, u8 value, u32 cycles_passed)
{
	UNUSED(cycles_passed);
	
	if (adress < 0x2000)
		ram_enabled = ((value & 0x0F) == 0x0A) && (ram != nullptr);

	else if (adress < 0x3000)
		switch_rom_bank((rom_bank & 0x100) | value);

	else if (adress < 0x4000)
		switch_rom_bank(change_bit(rom_bank, value & 1, 8));

	else if (adress < 0x6000)
		switch_bank_ram(value & 0x0F);

	else if (adress >= 0xA000 && adress < 0xC000)
	{
		const u32 real_address = adress - 0xA000 + (ram_bank * 0x2000);

		if (ram_enabled && real_address < ram_size)
			ram[real_address] = value;
	}
}

const u8* MBC5::get_dma_ptr(u16 adress)
{
	if (adress < 0x4000)
		return &rom[adress];

	else if (adress < 0x8000)
		return &rom[adress - 0x4000 + (rom_bank * 0x4000)];

	else if (adress >= 0xA000 && adress < 0xC000 && ram_enabled)
		return &ram[adress - 0xA000 + (ram_bank * 0x2000)];

	else
		return nullptr;
}


