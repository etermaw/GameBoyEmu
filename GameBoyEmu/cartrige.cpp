#include "stdafx.h"
#include "cartrige.h"

struct rom_header
{
	u8 start_vector[4]; 
	u8 nintendo_logo[48];
	u8 game_title[11];
	u8 manufacturer_code[4];
	u8 cgb_flag;
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
	const rom_header* header = reinterpret_cast<rom_header*>(&rom[0x100]);

	if (battery_ram)
	{
		std::ofstream ram_file(file_name + "_ram", std::ios::trunc | std::ios::binary);
		ram_file.write(reinterpret_cast<char*>(ram.get()), ram_size);
	}

	if (in_range(header->cartrige_type, 0x0F, 0x10))
	{
		std::ofstream rtc_file(file_name + "_rtc", std::ios::trunc);

		memory_interface.reset(); //make sure that MBC3 update rtc_regs

		auto epoch = std::chrono::system_clock::now().time_since_epoch();
		auto cur_timestamp = std::chrono::duration_cast<std::chrono::seconds>(epoch);

		rtc_file << cur_timestamp.count();
		rtc_file.write(reinterpret_cast<char*>(rtc_regs), sizeof(u8) * 5);
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
			ram_file.read(reinterpret_cast<char*>(ram.get()), ram_size);
	}

	if (in_range(header->cartrige_type, 0x0F, 0x10))
	{
		std::ifstream rtc_file(file_name + "_rtc", std::ios::binary);

		if (rtc_file.is_open())
		{
			i64 saved_timestamp;
			rtc_file >> saved_timestamp;
			rtc_file.read(reinterpret_cast<char*>(rtc_regs), sizeof(u8) * 5);

			auto epoch = std::chrono::system_clock::now().time_since_epoch();
			auto cur_timestamp = std::chrono::duration_cast<std::chrono::seconds>(epoch);
			auto delta = cur_timestamp.count() - saved_timestamp;

			if (delta <= 0 || check_bit(rtc_regs[4], 6))
				return;

			auto ns = rtc_regs[0] + delta;
			auto nm = rtc_regs[1] + ns / 60;
			auto nh = rtc_regs[2] + nm / 60;
			auto nd = (((rtc_regs[4] & 1) << 8) | rtc_regs[3]) + nh / 24;

			rtc_regs[0] = ns % 60;
			rtc_regs[1] = nm % 60;
			rtc_regs[2] = nh % 24;
			rtc_regs[3] = (nd % 512) & 0xFF;
			rtc_regs[4] = change_bit(rtc_regs[4], (nd % 512) > 255, 0);
			rtc_regs[4] = change_bit(rtc_regs[4], nd > 511, 7);
		}
	}
}

bool Cartrige::load_cartrige(const std::string& name)
{
	std::ifstream cart_file(name, std::ios::binary | std::ios::ate);

	if (!cart_file.is_open())
		return false;

	size_t size = cart_file.tellg();
	cart_file.seekg(0, std::ios_base::beg);

	rom = std::make_unique<u8[]>(size);
	cart_file.read(reinterpret_cast<char*>(rom.get()), size);

	file_name = name;
	load_ram();
	dispatch();

	return true;
}

IMemory* Cartrige::get_memory_controller() const
{
	return memory_interface.get();
}

IDmaMemory* Cartrige::get_dma_controller() const
{
	return dma_interface;
}

bool Cartrige::is_cgb_ready() const
{
	const rom_header* header = reinterpret_cast<rom_header*>(&rom[0x100]);

	return (header->cgb_flag == 0x80) || (header->cgb_flag == 0xC0);
}

void Cartrige::serialize(std::ostream& stream)
{
	//TODO: update rtc regs before serialization
	stream << battery_ram << ram_size;
	stream.write(reinterpret_cast<char*>(ram.get()), sizeof(u8) * ram_size);
	stream.write(reinterpret_cast<char*>(rtc_regs), sizeof(u8) * 5);
}

void Cartrige::deserialize(std::istream & stream)
{
	stream >> battery_ram >> ram_size;
	stream.read(reinterpret_cast<char*>(ram.get()), sizeof(u8) * ram_size);
	stream.read(reinterpret_cast<char*>(rtc_regs), sizeof(u8) * 5);
}

void Cartrige::dispatch()
{
	rom_header* header = reinterpret_cast<rom_header*>(&rom[0x100]);
	u8 type = header->cartrige_type;

	if (type == 0x00 || type == 0x08 || type == 0x09)
	{
		auto tmp = std::make_unique<NoMBC>(NoMBC(rom.get(), ram.get()));
		dma_interface = tmp.get();
		memory_interface = std::move(tmp);
	}

	else if (in_range(type, 0x01, 0x03))
	{
		auto tmp = std::make_unique<MBC1>(MBC1(rom.get(), ram.get()));
		dma_interface = tmp.get();
		memory_interface = std::move(tmp);
	}

	else if (in_range(type, 0x05, 0x06))
	{
		auto tmp = std::make_unique<MBC2>(MBC2(rom.get(), ram.get()));
		dma_interface = tmp.get();
		memory_interface = std::move(tmp);
	}

	else if (in_range(type, 0x0F, 0x13))
	{
		auto tmp = std::make_unique<MBC3>(MBC3(rom.get(), ram.get(), (type <= 0x10 ? rtc_regs : nullptr)));
		dma_interface = tmp.get();
		memory_interface = std::move(tmp);
	}

	else if (in_range(type, 0x1A, 0x1E))
	{
		auto tmp = std::make_unique<MBC5>(MBC5(rom.get(), ram.get()));
		dma_interface = tmp.get();
		memory_interface = std::move(tmp);
	}

	else
		memory_interface = nullptr;
}
