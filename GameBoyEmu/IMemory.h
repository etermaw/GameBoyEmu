#pragma once
#include "stdafx.h"

struct IMemory
{
	virtual ~IMemory() {}

	virtual u8 read_byte(u16 adress, u32 cycles_passed) = 0;
	virtual void write_byte(u16 adress, u8 value, u32 cycles_passed) = 0;
};