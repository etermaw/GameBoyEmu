#pragma once
#include "stdafx.h"
#include "IMemory.h"

class Timer final : public IMemory
{
	private:
		u32 divider_cycles;
		u32 counter_cycles;

		u8 control;
		u8 divider;
		u8 counter;
		u8 mod;
		
		bool enabled;

	public:
		Timer() : divider_cycles(0), counter_cycles(0), control(0), divider(0),
			counter(0), mod(0), enabled(false) {}

		bool step(u32 cycles);

		u8 read_byte(u16 adress) override;
		void write_byte(u16 adress, u8 value) override;
};