#pragma once
#include "stdafx.h"
#include "IMemory.h"

class MMU 
{
	private:
		std::vector<std::pair<u16, IMemory*>> chunks;
		function<void(u16,u8)> debug_write_callback;

		IMemory* find_chunk(u16 adress);

	public:
		void register_chunk(u16 start, u16 end, IMemory* handler);

		void attach_debug_callback(function<void(u16, u8)> callback) { debug_write_callback = callback; }

		u8 read_byte(u16 adress, u32 cycles_passed);
		void write_byte(u16 adress, u8 value, u32 cycles_passed);
};

