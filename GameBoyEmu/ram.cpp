#include "ram.h"

u8 Ram::read_byte(u16 adress, u32 cycles_passed)
{
	if (adress >= 0xC000 && adress < 0xD000)
		return memory[adress - 0xC000];

	else if (adress >= 0xD000 && adress < 0xE000)
		return memory[adress - 0xD000 + bank_num * 0x1000];

	else if (adress >= 0xE000 && adress < 0xF000)
		return memory[adress - 0xE000];

	else if (adress >= 0xF000 && adress < 0xFE00)
		return memory[adress - 0xF000 + bank_num * 0x1000];

	else if (cgb_mode && adress == 0xFF70)
		return bank_num;

	else if (adress >= 0xFF80 && adress < 0xFFFF)
		return high_ram[adress - 0xFF80];

	else
		return 0xFF;
}

void Ram::write_byte(u16 adress, u8 value, u32 cycles_passed)
{
	if (adress >= 0xC000 && adress < 0xD000)
		memory[adress - 0xC000] = value;

	else if (adress >= 0xD000 && adress < 0xE000)
		memory[adress - 0xD000 + bank_num * 0x1000] = value;

	else if (adress >= 0xE000 && adress < 0xF000)
		memory[adress - 0xE000] = value;

	else if (adress >= 0xF000 && adress < 0xFE00)
		memory[adress - 0xF000 + bank_num * 0x1000] = value;

	else if (cgb_mode && adress == 0xFF70)
		bank_num = std::max(1, value & 0x7);

	else if (adress >= 0xFF80 && adress < 0xFFFF)
		high_ram[adress - 0xFF80] = value;

	//else ignore it
}

const u8* Ram::get_dma_ptr(u16 adress)
{
	if (adress >= 0xC000 && adress < 0xD000)
		return &memory[adress - 0xC000];

	else if (adress >= 0xD000 && adress < 0xE000)
		return &memory[adress - 0xD000 + bank_num * 0x1000];

	else
		return nullptr;
}

void Ram::serialize(std::ostream& stream)
{
	stream.write(reinterpret_cast<char*>(memory.get()), sizeof(u8) * 0x8000);
	stream.write(reinterpret_cast<char*>(high_ram), sizeof(u8) * 0x80);
	stream << bank_num << cgb_mode;
}

void Ram::deserialize(std::istream& stream)
{
	stream.read(reinterpret_cast<char*>(memory.get()), sizeof(u8) * 0x8000);
	stream.read(reinterpret_cast<char*>(high_ram), sizeof(u8) * 0x80);
	stream >> bank_num >> cgb_mode;
}

