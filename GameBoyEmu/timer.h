#pragma once
#include "stdafx.h"
#include "IMemory.h"
#include "interrupts.h"

class Timer final : public IMemory
{
	private:
		Interrupts& interrupts;

		u32 cycles_ahead;
		u32 divider_cycles;
		u32 counter_cycles;

		u8 control;
		u8 divider;
		u8 counter;
		u8 mod;
		
		bool enabled;

		void step_ahead(u32 cycles);

	public:
		Timer(Interrupts& ints) : 
			cycles_ahead(0), divider_cycles(0), counter_cycles(0), control(0), divider(0),
			counter(0), mod(0), enabled(false), interrupts(ints) {}

		void step(u32 cycles);

		u8 read_byte(u16 adress, u32 cycles_passed) override;
		void write_byte(u16 adress, u8 value, u32 cycles_passed) override;

		void serialize(std::ostream& stream);
		void deserialize(std::istream& stream);
};