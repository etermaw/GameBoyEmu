#pragma once
#include "stdafx.h"
#include "IMemory.h"

class MMU 
{
	private:
		std::vector<std::pair<u16, IMemory*>> chunks;

		IMemory* find_chunk(u16 adress);

	public:
		void register_chunk(u16 start, u16 end, IMemory* handler);

		u8 read_byte(u16 adress);
		void write_byte(u16 adress, u8 value);
};

