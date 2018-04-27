#pragma once
//#include "stdafx.h"
#include "IMemory.h"
#include "interrupts.h"

class Serial : public IMemory
{
	enum SERIAL_STATE : i32 { SERIAL_SEND_INTERNAL, SERIAL_SEND_EXTERNAL, SERIAL_BURN_CYCLES };

	private:
		function<std::pair<u8, u32>(std::pair<u8, u32>)> send_receive_byte;

		Interrupts& int_handler;
		SERIAL_STATE state;
		i32 transfer_cycles;
		u8 reg, ctrl;
		bool transfer_enabled;
		bool cgb_mode;
		bool double_speed;

	public:
		Serial(Interrupts& ints);

		u8 read_byte(u16 adress, u32 cycles_passed) override;
		void write_byte(u16 adress, u8 value, u32 cycles_passed) override;

		void step(u32 cycles_passed);
		void enable_cgb_mode(bool mode);
		void set_speed(bool speed);

		void attach_callback(function<std::pair<u8, u32>(std::pair<u8, u32>)> callback);
		void serialize(std::ostream& stream);
		void deserialize(std::istream& stream);
};