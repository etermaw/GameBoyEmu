#pragma once
#include "stdafx.h"
#include "IMemory.h"

class Ram final : public IMemory
{
	private:
		u32 bank_num;

		std::unique_ptr<u8[]> memory;
		u8 high_ram[0x80];

		bool cgb_mode;

	public:
		Ram() : memory(std::make_unique<u8[]>(0x8000)), bank_num(1), cgb_mode(false) {}

		const u8* get() const { return memory.get(); }
		void enable_cgb_mode(bool enable_cgb) { cgb_mode = enable_cgb; }

		u8 read_byte(u16 adress, u32 cycles_passed) override;
		void write_byte(u16 adress, u8 value, u32 cycles_passed) override;
};