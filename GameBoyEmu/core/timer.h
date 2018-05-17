#pragma once

#include "IMemory.h"
#include "interrupts.h"

class Timer final : public IMemory
{
	private:
		Interrupts& interrupts;

		u32 cycles_ahead = 0;
		u32 divider_cycles = 0;
		u32 counter_cycles = 0;

		u8 control = 0;
		u8 divider = 0;
		u8 counter = 0;
		u8 mod = 0;
		
		bool enabled = false;
		bool overflow = false;
		bool reload = false;

		void step_ahead(u32 cycles);
		void check_fault_bits();

	public:
		Timer(Interrupts& ints) : interrupts(ints) {}

		void step(u32 cycles);

		u8 read_byte(u16 adress, u32 cycles_passed) override;
		void write_byte(u16 adress, u8 value, u32 cycles_passed) override;

		void reset();
		void serialize(std::ostream& stream);
		void deserialize(std::istream& stream);
};