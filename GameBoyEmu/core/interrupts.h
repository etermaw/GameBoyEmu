#pragma once

#include "IMemory.h"

enum INTERRUPTS { INT_VBLANK, INT_LCD, INT_TIMER, INT_SERIAL, INT_JOYPAD, INT_NONE };

class Interrupts final : public IMemory
{
	private:
		u8 interrupt_mask = 0;
		u8 interrupt_flags = 0;

	public:
		void raise(INTERRUPTS code);
		bool is_any_raised();
		INTERRUPTS get_first_raised();

		u8 read_byte(u16 adress, u32 cycles_passed) override;
		void write_byte(u16 adress, u8 value, u32 cycles_passed) override;
		
		void reset();
		void serialize(std::ostream& stream);
		void deserialize(std::istream& stream);
};