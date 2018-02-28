#pragma once
#include "stdafx.h"
#include "IMemory.h"

class MBCBase
{
	protected:
		const u8* rom;
		u8* ram;

		u32 rom_bank;
		u32 ram_bank;
		bool ram_enabled;
		
	public:
		MBCBase(const u8* rom = nullptr, u8* ram = nullptr) : 
			rom(rom), ram(ram), rom_bank(1), ram_bank(0), ram_enabled(false) {}
};

class NoMBC final : public IMemory, public IDmaMemory
{
	private:
		const u8* rom;
		u8* ram;
		bool ram_enabled;

	public:
		NoMBC(const u8* rom = nullptr, u8* ram = nullptr) : rom(rom), ram(ram), ram_enabled(false) {}

		u8 read_byte(u16 adress, u32 cycles_passed) override;
		void write_byte(u16 adress, u8 value, u32 cycles_passed) override;

		const u8* get_dma_ptr(u16 adress) override;
};

class MBC1 final : public MBCBase, public IMemory, public IDmaMemory
{
	private:
		u8 rom_num_high;
		u8 rom_num_low;

		bool ram_mode;

	public:
		MBC1(const u8* rom, u8* ram) : 
			MBCBase(rom, ram), rom_num_high(0), rom_num_low(1), ram_mode(false) {}

		u8 read_byte(u16 adress, u32 cycles_passed) override;
		void write_byte(u16 adress, u8 value, u32 cycles_passed) override;

		const u8* get_dma_ptr(u16 adress) override;
};

class MBC2 final : public MBCBase, public IMemory, public IDmaMemory
{
	public:
		MBC2(const u8* rom, u8* ram) : MBCBase(rom, ram) {}

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
		u8 last_write;
		bool reg_used = false;

		void latch_rtc();
		void update_time();

	public:
		MBC3(const u8* rom = nullptr, u8* ram = nullptr, u8* rtc_regs = nullptr) : 
			MBCBase(rom, ram), start_time(std::chrono::system_clock::now()), rtc(rtc_regs) {}

		~MBC3() { if(rtc) update_time(); }

		u8 read_byte(u16 adress, u32 cycles_passed) override;
		void write_byte(u16 adress, u8 value, u32 cycles_passed) override;

		const u8* get_dma_ptr(u16 adress) override;
};

class MBC5 final : public MBCBase, public IMemory, public IDmaMemory
{
	public:
		MBC5(const u8* rom, u8* ram) : MBCBase(rom, ram) {}

		u8 read_byte(u16 adress, u32 cycles_passed) override;
		void write_byte(u16 adress, u8 value, u32 cycles_passed) override;

		const u8* get_dma_ptr(u16 adress) override;
};

