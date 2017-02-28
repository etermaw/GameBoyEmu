#pragma once
#include "stdafx.h"
#include "IMemory.h"

class MBCBase
{
	protected:
		const u8* rom;
		const u8* cur_rom_bank;

		u8* ram;
		u8* cur_ram_bank;

		bool ram_enabled;
		
		void swap_rom_bank(u32 new_bank_num);
		void swap_ram_bank(u32 new_bank_num);

	public:
		MBCBase(const u8* rom, u8* ram) : rom(rom), ram(ram), 
			ram_enabled(ram ? true : false) 
		{
			cur_rom_bank = rom;
			cur_ram_bank = ram;
		}
};

class ROMOnly final : public IMemory
{
	private:
		const u8* memory;

	public:
		ROMOnly(const u8* rom) : memory(rom) {}

		u8 read_byte(u16 adress) override;
		void write_byte(u16 adress, u8 value) override;
};

class MBC1 final : public MBCBase, public IMemory
{
	private:
		u8 rom_num_high;
		u8 rom_num_low;

		bool ram_mode;

	public:
		MBC1(const u8* rom, u8* ram) : MBCBase(rom, ram),
			rom_num_high(0),rom_num_low(1),ram_mode(ram ? true : false) 
		{
			swap_rom_bank(1);
		}

		u8 read_byte(u16 adress) override;
		void write_byte(u16 adress, u8 value) override;
};

class MBC2 final : public MBCBase, public IMemory
{
	public:
		MBC2(const u8* rom, u8* ram) : MBCBase(rom, ram) {}

		u8 read_byte(u16 adress) override;
		void write_byte(u16 adress, u8 value) override;
};

class MBC3 final : public MBCBase, public IMemory
{
	private:
		u8* time_reg;
		u8* rtc;
		u8 last_write;
		bool reg_used;

		void latch_rtc();

	public:
		MBC3(const u8* rom, u8* ram, u8* rtc_regs) : MBCBase(rom, ram), rtc(rtc_regs) {}

		u8 read_byte(u16 adress) override;
		void write_byte(u16 adress, u8 value) override;
};

class MBC5 final : public MBCBase, public IMemory
{
	private:
		u8 rom_num_high;
		u8 rom_num_low;

	public:
		MBC5(const u8* rom, u8* ram) : MBCBase(rom, ram) {}

		u8 read_byte(u16 adress) override;
		void write_byte(u16 adress, u8 value) override;
};

