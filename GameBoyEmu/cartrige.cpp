#include "stdafx.h"
#include "cartrige.h"

struct rom_header
{
	u8 start_vector[4]; 
	u8 nintendo_logo[48];
	u8 game_title[11];
	u8 manufacturer_code[4];
	u8 gbc_flag;
	u8 new_license_code[2];
	u8 sgb_flag;
	u8 cartrige_type;
	u8 rom_size;
	u8 ram_size;
	u8 destination_code;
	u8 old_license_code;
	u8 rom_version;
	u8 checksum;
	u8 global_checksum[2];
};

bool in_range(u32 value, u32 begin, u32 end)
{
	return (value >= begin) && (value <= end);
}

u32 get_ram_size(u8 val)
{
	assert(val < 6);

	static const u32 sizes[] = { 0, 0x800, 0x2000, 0x8000, 0x20000, 0x10000 };

	return sizes[val];
}

Cartrige::~Cartrige()
{
	if (battery_ram)
	{
		std::ofstream ram_file(file_name + "_ram", std::ios::trunc | std::ios::binary);
		rom_header* header = reinterpret_cast<rom_header*>(&rom[0x100]);

		if (in_range(header->rom_version, 0x0F, 0x10))
			ram_file.write(reinterpret_cast<char*>(rtc_regs), 5);

		ram_file.write(reinterpret_cast<char*>(ram.get()), ram_size);
	}
}

void Cartrige::load_ram()
{
	rom_header* header = reinterpret_cast<rom_header*>(&rom[0x100]);

	//MBC2 has always header->ram_size == 0, but it has 512 bytes actually!
	if (in_range(header->cartrige_type, 0x05, 0x06))
		ram_size = 512;

	else
		ram_size = get_ram_size(header->ram_size);

	ram = ram_size ? std::make_unique<u8[]>(ram_size) : nullptr;

	switch (header->cartrige_type)
	{
		case 0x03:
		case 0x06:
		case 0x09:
		case 0x0D:
		case 0x0F:
		case 0x10:
		case 0x13:
		case 0x1B:
		case 0x1E:
			battery_ram = true;
			break;

		default:
			battery_ram = false;
			break;
	}

	if (battery_ram)
	{
		std::ifstream ram_file(file_name + "_ram", std::ios::binary);

		if (ram_file.is_open())
		{
			//if this is mbc3 cart, we need to save additional time registers
			if (in_range(header->cartrige_type, 0x0F, 0x10))
				ram_file.read(reinterpret_cast<char*>(rtc_regs), 5); 

			ram_file.read(reinterpret_cast<char*>(ram.get()), ram_size);
		}
	}
}

void Cartrige::load_cartrige(const std::string& name)
{
	std::ifstream cart_file(name, std::ios::binary | std::ios::ate);

	if (!cart_file.is_open())
		return;

	size_t size = cart_file.tellg();
	cart_file.seekg(0, std::ios_base::beg);

	rom = std::make_unique<u8[]>(size);
	cart_file.read(reinterpret_cast<char*>(rom.get()), size);

	file_name = name;
	load_ram();

	dispatch();
}

IMemory* Cartrige::get_memory_interface() const
{
	return memory_interface.get();
}

void Cartrige::dispatch()
{
	rom_header* header = reinterpret_cast<rom_header*>(&rom[0x100]);

	if (header->cartrige_type == 0x00)
		memory_interface = std::make_unique<ROMOnly>(ROMOnly(rom.get()));

	else if (in_range(header->cartrige_type, 0x01, 0x03))
		memory_interface = std::make_unique<MBC1>(MBC1(rom.get(), ram.get()));

	else if (in_range(header->cartrige_type, 0x05, 0x06))
		memory_interface = std::make_unique<MBC2>(MBC2(rom.get(), ram.get()));

	else if (in_range(header->cartrige_type, 0x0F, 0x13))
		memory_interface = std::make_unique<MBC3>(MBC3(rom.get(), ram.get(), rtc_regs));

	else if (in_range(header->cartrige_type, 0x1A, 0x1E))
		memory_interface = std::make_unique<MBC5>(MBC5(rom.get(), ram.get()));

	else
		memory_interface = nullptr;
}
