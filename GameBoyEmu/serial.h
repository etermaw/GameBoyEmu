#pragma once
#include "stdafx.h"
#include "IMemory.h"
#include "interrupts.h"

class Serial : public IMemory
{
	enum SERIAL_STATE { SERIAL_SEND_INTERNAL, SERIAL_SEND_EXTERNAL, SERIAL_BURN_CYCLES };

	private:
		function<std::pair<u8, u32>(std::pair<u8, u32>)> send_receive_byte;

		Interrupts& int_handler;
		SERIAL_STATE state;
		u32 transfer_cycles;
		u8 reg, ctrl;
		bool transfer_enabled;
		bool cgb_mode;

	public:
		Serial(Interrupts& ints);

		u8 read_byte(u16 adress, u32 cycles_passed) override;
		void write_byte(u16 adress, u8 value, u32 cycles_passed) override;

		void step(u32 cycles_passed);
};