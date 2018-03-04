#pragma once
#include "stdafx.h"
#include "IMemory.h"

class MBCBase
{
	protected:
		const u8* rom = nullptr;
		u8* ram = nullptr;

		u32 rom_bank = 1;
		u32 ram_bank = 0;
		u32 max_rom_banks = 0;
		bool ram_enabled = false;

		//swapped names, because single letter difference is not enough
		void switch_rom_bank(u32 new_bank_rom);
		void switch_bank_ram(u32 new_ram_bank);
		
	public:
		MBCBase(const u8* rom, u8* ram, u32 rom_banks) :  rom(rom), ram(ram), max_rom_banks(rom_banks) {}
};

class NoMBC final : public IMemory, public IDmaMemory
{
	private:
		const u8* rom;
		u8* ram;
		bool ram_enabled = false;

	public:
		NoMBC(const u8* rom = nullptr, u8* ram = nullptr) : rom(rom), ram(ram) {}

		u8 read_byte(u16 adress, u32 cycles_passed) override;
		void write_byte(u16 adress, u8 value, u32 cycles_passed) override;

		const u8* get_dma_ptr(u16 adress) override;
};

class MBC1 final : public MBCBase, public IMemory, public IDmaMemory
{
	private:
		u8 rom_num_high = 0;
		u8 rom_num_low = 1;

		bool ram_mode = false;

	public:
		MBC1(const u8* rom, u8* ram, u32 rom_banks) : MBCBase(rom, ram, rom_banks) {}

		u8 read_byte(u16 adress, u32 cycles_passed) override;
		void write_byte(u16 adress, u8 value, u32 cycles_passed) override;

		const u8* get_dma_ptr(u16 adress) override;
};

class MBC2 final : public MBCBase, public IMemory, public IDmaMemory
{
	public:
		MBC2(const u8* rom, u8* ram, u32 rom_banks) : MBCBase(rom, ram, rom_banks) {}

		u8 read_byte(u16 adress, u32 cycles_passed) override;
		void write_byte(u16 adress, u8 value, u32 cycles_passed) override;

		const u8* get_dma_ptr(u16 adress) override;
};

class MBC3 final : public MBCBase, public IMemory, public IDmaMemory
{
	using time_point = std::chrono::time_point<std::chrono::system_clock>;

	private:
		time_point start_time;
		u8* rtc;
		u8 latched_rtc[5] = {};
		u8 selected_time_reg = 0;
		u8 last_write = 0;
		bool reg_used = false;

		void latch_rtc();
		void update_time();

	public:
		MBC3(const u8* rom = nullptr, u8* ram = nullptr, u8* rtc_regs = nullptr, u32 rom_banks = 0) :
			MBCBase(rom, ram, rom_banks), start_time(std::chrono::system_clock::now()), rtc(rtc_regs) {}

		~MBC3() { if(rtc) update_time(); }

		u8 read_byte(u16 adress, u32 cycles_passed) override;
		void write_byte(u16 adress, u8 value, u32 cycles_passed) override;

		const u8* get_dma_ptr(u16 adress) override;
};

class MBC5 final : public MBCBase, public IMemory, public IDmaMemory
{
	public:
		MBC5(const u8* rom, u8* ram, u32 rom_banks) : MBCBase(rom, ram, rom_banks) {}

		u8 read_byte(u16 adress, u32 cycles_passed) override;
		void write_byte(u16 adress, u8 value, u32 cycles_passed) override;

		const u8* get_dma_ptr(u16 adress) override;
};
