#include "apu.h"

APU::APU() : dummy_regs()
{
	static const u8 init_wave_ram[] = { 0x84,0x40,0x43,0xAA,0x2D,0x78,0x92,0x3C,0x60,0x59,0x59,0xB0,0x34,0xB8,0x2E,0xDA };

	std::memcpy(wave_ram, init_wave_ram, sizeof(u8) * 16);
}

u8 APU::read_byte(u16 adress)
{
	if (adress >= 0xFF10 && adress <= 0xFF26)
		return dummy_regs[adress - 0xFF10];

	else if (adress >= 0xFF30 && adress <= 0xFF3F)
		return wave_ram[adress - 0xFF30];

	else
		return 0xFF;
}

void APU::write_byte(u16 adress, u8 value)
{
	if (adress >= 0xFF10 && adress <= 0xFF26)
		dummy_regs[adress - 0xFF10] = value;

	else if (adress >= 0xFF30 && adress <= 0xFF3F)
		wave_ram[adress - 0xFF30] = value;

	//else ignore
}

void APU::step(u32 cycles)
{
}
