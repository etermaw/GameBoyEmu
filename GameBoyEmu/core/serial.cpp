////#include "stdafx.h"
#include "serial.h"

Serial::Serial(Interrupts& ints) : int_handler(ints), cgb_mode(false), double_speed(false) {}

u8 Serial::read_byte(u16 adress, u32 cycles_passed)
{
	UNUSED(cycles_passed);
	
	if (adress == 0xFF01)
		return reg;

	else if (adress == 0xFF02)
		return change_bit(ctrl, transfer_enabled, 7);

	else
		return 0xFF;
}

void Serial::write_byte(u16 adress, u8 value, u32 cycles_passed)
{
	UNUSED(cycles_passed);
	
	if (adress == 0xFF01)
		reg = value;

	else if (adress == 0xFF02)
	{
		transfer_enabled = check_bit(value, 7);

		if (transfer_enabled)
			state = check_bit(value, 0) ? SERIAL_SEND_INTERNAL : SERIAL_SEND_EXTERNAL;

		transfer_cycles = ((cgb_mode && check_bit(value, 1)) ? 64 : 4096) >> double_speed; //cycles to transfer 1 byte

		ctrl = value;
	}
}

void Serial::step(u32 cycles_passed)
{	
	if (transfer_enabled)
	{
		switch (state)
		{
			default:
				if (state == SERIAL_SEND_INTERNAL)
					std::tie(reg, std::ignore) = send_receive_byte({reg, transfer_cycles});

				else
				{
					std::tie(reg, transfer_cycles) = send_receive_byte({ reg, -1U });
					transfer_cycles <<= double_speed; //adjust cycles to serial port speed
				}
				
				state = SERIAL_BURN_CYCLES;

			case SERIAL_BURN_CYCLES:
				transfer_cycles -= cycles_passed;

				if (transfer_cycles <= 0)
				{
					transfer_enabled = false;
					int_handler.raise(INT_SERIAL);
				}

				//TODO: add shifting bits per serial clock shift rate!

				break;
		}
	}
}

void Serial::enable_cgb_mode(bool mode)
{
	cgb_mode = mode;
}

void Serial::set_speed(bool speed)
{
	double_speed = speed;
}

void Serial::attach_callback(function<std::pair<u8, u32>(std::pair<u8, u32>)> callback)
{
	send_receive_byte = callback;
}

void Serial::serialize(std::ostream& stream)
{
	i32 tmp_state = static_cast<i32>(state);

	stream << tmp_state << transfer_cycles << reg << ctrl;
	stream << transfer_enabled << cgb_mode << double_speed;
}

void Serial::deserialize(std::istream& stream)
{
	i32 tmp_state;

	stream >> tmp_state >> transfer_cycles >> reg >> ctrl;
	stream >> transfer_enabled >> cgb_mode >> double_speed;

	state = static_cast<SERIAL_STATE>(tmp_state);
}
