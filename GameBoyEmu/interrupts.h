#pragma once
#include "stdafx.h"
#include "IMemory.h"

class Interrupts final : public IMemory
{
	private:
		u8 interrupt_mask;
		u8 interrupt_flags;

	public:
		Interrupts() : interrupt_flags(0), interrupt_mask(0) {}

		void raise(INTERRUPTS code);
		bool is_any_raised();
		INTERRUPTS get_first_raised();

		u8 read_byte(u16 adress) override;
		void write_byte(u16 adress, u8 value) override;
};