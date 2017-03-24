#pragma once
#include "stdafx.h"
#include "IMemory.h"

class MBCBase
{
	protected:
		const u8* rom;
		u8* ram;

		const u8* cur_rom_bank;
		u8* cur_ram_bank;

		bool ram_enabled;
		
		void swap_rom_bank(u32 new_bank_num);
		void swap_ram_bank(u32 new_bank_num);

	public:
		MBCBase(const u8* rom = nullptr, u8* ram = nullptr) : 
			rom(rom), ram(ram), cur_rom_bank(rom), cur_ram_bank(ram), ram_enabled(false) {}
};

class NoMBC final : public IMemory
{
	private:
		const u8* rom;
		u8* ram;
		bool ram_enabled;

	public:
		NoMBC(const u8* rom = nullptr, u8* ram = nullptr) : rom(rom), ram(ram), ram_enabled(false) {}

		u8 read_byte(u16 adress, u32 cycles_passed) override;
		void write_byte(u16 adress, u8 value, u32 cycles_passed) override;
};

class MBC1 final : public MBCBase, public IMemory
{
	private:
		u8 rom_num_high;
		u8 rom_num_low;

		bool ram_mode;

	public:
		MBC1(const u8* rom, u8* ram) : 
			MBCBase(rom, ram), rom_num_high(0), rom_num_low(1), ram_mode(false) 
		{
			swap_rom_bank(1);
		}

		u8 read_byte(u16 adress, u32 cycles_passed) override;
		void write_byte(u16 adress, u8 value, u32 cycles_passed) override;
};

class MBC2 final : public MBCBase, public IMemory
{
	public:
		MBC2(const u8* rom, u8* ram) : MBCBase(rom, ram) {}

		u8 read_byte(u16 adress, u32 cycles_passed) override;
		void write_byte(u16 adress, u8 value, u32 cycles_passed) override;
};

class MBC3 final : public MBCBase, public IMemory
{
	private:
		u8* rtc;
		u8 selected_time_reg;
		u8 last_write;
		bool reg_used;

		void latch_rtc();

	public:
		MBC3(const u8* rom = nullptr, u8* ram = nullptr, u8* rtc_regs = nullptr) : 
			MBCBase(rom, ram), rtc(rtc_regs), reg_used(false), selected_time_reg(0) {}

		u8 read_byte(u16 adress, u32 cycles_passed) override;
		void write_byte(u16 adress, u8 value, u32 cycles_passed) override;
};

class MBC5 final : public MBCBase, public IMemory
{
	private:
		u8 rom_num_high;
		u8 rom_num_low;

	public:
		MBC5(const u8* rom, u8* ram) : MBCBase(rom, ram) {}

		u8 read_byte(u16 adress, u32 cycles_passed) override;
		void write_byte(u16 adress, u8 value, u32 cycles_passed) override;
};

