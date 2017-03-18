#pragma once
#include "stdafx.h"
#include "IMemory.h"

class APU final : public IMemory
{
	private:
		/*u8 nr1x[5];
		u8 nr2x[4];
		u8 nr3x[5];
		u8 nr4x[4];
		u8 control[3];*/
		u8 wave_ram[16];
		u8 dummy_regs[22];

	public:
		APU();
		
		u8 read_byte(u16 adress) override;
		void write_byte(u16 adress, u8 value) override;

		void step(u32 cycles);
};