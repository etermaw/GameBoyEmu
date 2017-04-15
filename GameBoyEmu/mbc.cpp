#include "stdafx.h"
#include "mbc.h"

void MBCBase::swap_rom_bank(u32 new_bank_num)
{
	cur_rom_bank = rom + new_bank_num * 0x4000;
}

void MBCBase::swap_ram_bank(u32 new_bank_num)
{
	cur_ram_bank = ram + new_bank_num * 0x2000;
}

u8 NoMBC::read_byte(u16 adress, u32 cycles_passed)
{
	if (adress < 0x8000)
		return rom[adress];

	else if (adress >= 0xA000 && adress < 0xC000)
		return ram_enabled ? ram[adress - 0xA000] : 0xFF;
}

void NoMBC::write_byte(u16 adress, u8 value, u32 cycles_passed)
{
	if (adress < 0x2000)
		ram_enabled = ((value & 0x0F) == 0x0A) && (ram != nullptr);

	else if (adress >= 0xA000 && adress < 0xC000 && ram_enabled)
		ram[adress - 0xA000] = value;

	//else ignore
}

u8 MBC1::read_byte(u16 adress, u32 cycles_passed)
{
	if (adress < 0x4000)
		return rom[adress];

	else if (adress < 0x8000)
		return cur_rom_bank[adress - 0x4000];

	else if (adress >= 0xA000 && adress < 0xC000)
		return (ram_enabled ? cur_ram_bank[adress - 0xA000] : 0xFF);
}

void MBC1::write_byte(u16 adress, u8 value, u32 cycles_passed)
{
	if (adress < 0x2000)
		ram_enabled = ((value & 0x0F) == 0x0A) && (ram != nullptr);

	else if (adress < 0x4000)
	{
		rom_num_low = std::max(1, value & 0x1F);
		swap_rom_bank(((ram_mode ? 0 : rom_num_high) << 5) | rom_num_low);
	}

	else if (adress < 0x6000)
	{
		if (ram_mode)
			swap_ram_bank(value & 0x03);

		else
		{
			rom_num_high = value & 0x3;
			swap_rom_bank((rom_num_high << 5) | rom_num_low);
		}
	}

	else if (adress < 0x8000)
	{
		ram_mode = static_cast<bool>(value & 1) && (ram != nullptr);

		if (ram_mode)
			swap_rom_bank(rom_num_low & 0x1F);

		else
		{
			swap_rom_bank((rom_num_high << 5) | rom_num_low);
			swap_ram_bank(0);
		}
	}

	else if (adress >= 0xA000 && adress < 0xC000 && ram_enabled)
		cur_ram_bank[adress - 0xA000] = value;
}

u8 MBC2::read_byte(u16 adress, u32 cycles_passed)
{
	if (adress < 0x4000)
		return rom[adress];

	else if (adress < 0x8000)
		return cur_rom_bank[adress - 0x4000];

	else if (adress >= 0xA000 && adress < 0xA200)
		return (ram_enabled ? ram[adress - 0xA000] : 0xFF);
}

void MBC2::write_byte(u16 adress, u8 value, u32 cycles_passed)
{
	if (adress < 0x2000 && ((adress & 0x0100) == 0x0000))
		ram_enabled = ((value & 0x0F) == 0x0A);

	else if (adress < 0x4000 && ((adress & 0x0100) == 0x0100))
		swap_rom_bank(value & 0x0F);

	else if (adress >= 0xA000 && adress < 0xA200 && ram_enabled)
		ram[adress - 0xA000] = value & 0xF;
}

void MBC3::latch_rtc()
{
}

u8 MBC3::read_byte(u16 adress, u32 cycles_passed)
{
	if (adress < 0x4000)
		return rom[adress];

	else if (adress < 0x8000)
		return cur_rom_bank[adress - 0x4000];

	else if (adress >= 0xA000 && adress < 0xC000 && ram_enabled)
		return reg_used ? rtc[selected_time_reg] : cur_ram_bank[adress - 0xA000];

	else
		return 0xFF;
}

void MBC3::write_byte(u16 adress, u8 value, u32 cycles_passed)
{
	if (adress < 0x2000)
		ram_enabled = ((value & 0x0F) == 0x0A); //ram_enabled affects ram AND timer

	else if (adress < 0x4000)
		swap_rom_bank(std::max(1, value & 0x7F));
	
	else if (adress < 0x6000)
	{
		if (value < 0x08) 
		{
			reg_used = false;
			swap_ram_bank(value);
		}

		else if (rtc != nullptr) //TODO: keep an eye on it
		{
			reg_used = true;
			selected_time_reg = value - 0x08;
		}
	}

	else if (adress < 0x8000)
	{
		if (last_write == 0x00 && value == 0x01)
			latch_rtc();

		last_write = value;
	}

	else if (adress >= 0xA000 && adress < 0xC000 && ram_enabled)
		reg_used ? rtc[selected_time_reg] : cur_ram_bank[adress - 0xA000] = value;
}

u8 MBC5::read_byte(u16 adress, u32 cycles_passed)
{
	if (adress < 0x4000)
		return rom[adress];

	else if (adress < 0x8000)
		return cur_rom_bank[adress - 0x4000];

	else if (adress >= 0xA000 && adress < 0xC000)
		return (ram_enabled ? cur_ram_bank[adress - 0xA000] : 0xFF);
}

void MBC5::write_byte(u16 adress, u8 value, u32 cycles_passed)
{
	if (adress < 0x2000)
		ram_enabled = ((value & 0x0F) == 0x0A) && (ram != nullptr);

	else if (adress < 0x3000)
	{
		rom_num_low = value;
		swap_rom_bank((rom_num_high << 8) | rom_num_low);
	}

	else if (adress < 0x4000)
	{
		rom_num_high = value & 0x01;
		swap_rom_bank((rom_num_high << 8) | rom_num_low);
	}

	else if (adress < 0x6000)
		swap_ram_bank(value & 0x0F);

	else if (adress >= 0xA000 && adress < 0xC000 && ram_enabled)
		cur_ram_bank[adress - 0xA000] = value;
}


