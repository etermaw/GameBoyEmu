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

void Cartrige::attach_rom(const u8* rom_ptr, u32 size)
{
	rom = std::make_pair(rom_ptr, size);
}

void Cartrige::attach_ram(u8* ram_ptr, u32 size)
{
	ram = std::make_pair(ram_ptr, size);
}

void Cartrige::attach_rtc(u8* rtc_ptr, u32 size)
{
	rtc2 = std::make_pair(rtc_ptr, size);
}

bool Cartrige::has_battery_ram() const
{
	if (std::get<0>(rom) == nullptr)
		return false;

	const rom_header* header = reinterpret_cast<const rom_header*>(&std::get<0>(rom)[0x100]);

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
			return true;

		default:
			return false;
	}
}

u32 Cartrige::get_declared_ram_size() const
{
	static const u32 sizes[] = { 0, 0x800, 0x2000, 0x8000, 0x20000, 0x10000 };

	if (std::get<0>(rom) == nullptr)
		return 0;

	const rom_header* header = reinterpret_cast<const rom_header*>(&std::get<0>(rom)[0x100]);

	//MBC2 has always header->ram_size == 0, but it has 512 bytes actually!
	if (in_range(header->cartrige_type, 0x05, 0x06))
		return 512;

	else
		return (header->ram_size > 5) ? 0 : sizes[header->ram_size];
}

bool Cartrige::has_rtc() const
{
	if (std::get<0>(rom) == nullptr)
		return false;

	const rom_header* header = reinterpret_cast<const rom_header*>(&std::get<0>(rom)[0x100]);
	return in_range(header->cartrige_type, 0xF, 0x10);
}

void Cartrige::setup()
{
	dispatch();
}

std::string Cartrige::get_name() const
{
    const rom_header* header = reinterpret_cast<const rom_header*>(&std::get<0>(rom)[0x100]);
    return std::string(std::begin(header->game_title), std::end(header->game_title));
}

void Cartrige::load_rtc(std::ifstream& rtc_file)
{
	if (has_rtc() && rtc_file.is_open())
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
	const rom_header* header = reinterpret_cast<const rom_header*>(&std::get<0>(rom)[0x100]);
	return (header->cgb_flag == 0x80) || (header->cgb_flag == 0xC0);
}

void Cartrige::reset()
{
	//TODO: what to do here?
}

void Cartrige::serialize(std::ostream& stream)
{
	//TODO: update rtc regs before serialization
	stream << std::get<1>(ram);
	stream.write(reinterpret_cast<char*>(std::get<0>(ram)), std::get<1>(ram));
	stream.write(reinterpret_cast<char*>(rtc_regs), sizeof(u8) * 5);
}

void Cartrige::deserialize(std::istream & stream)
{
	stream >> std::get<1>(ram);
	stream.read(reinterpret_cast<char*>(std::get<0>(ram)), std::get<1>(ram));
	stream.read(reinterpret_cast<char*>(rtc_regs), sizeof(u8) * 5);
}

void Cartrige::dispatch()
{
	const u8* rom_ptr = std::get<0>(rom);
	const rom_header* header = reinterpret_cast<const rom_header*>(&rom_ptr[0x100]);
	const u8 type = header->cartrige_type;
	const u32 rom_banks = 2 << header->rom_size;

	if (type == 0x00 || type == 0x08 || type == 0x09)
	{
		auto tmp = std::make_unique<NoMBC>(NoMBC(rom_ptr, std::get<0>(ram), std::get<1>(ram)));
		dma_interface = tmp.get();
		memory_interface = std::move(tmp);
	}

	else if (in_range(type, 0x01, 0x03))
	{
		auto tmp = std::make_unique<MBC1>(MBC1(rom_ptr, std::get<0>(ram), rom_banks, std::get<1>(ram)));
		dma_interface = tmp.get();
		memory_interface = std::move(tmp);
	}

	else if (in_range(type, 0x05, 0x06))
	{
		auto tmp = std::make_unique<MBC2>(MBC2(rom_ptr, std::get<0>(ram), rom_banks, std::get<1>(ram)));
		dma_interface = tmp.get();
		memory_interface = std::move(tmp);
	}

	else if (in_range(type, 0x0F, 0x13))
	{
		auto tmp = std::make_unique<MBC3>(MBC3(rom_ptr, std::get<0>(ram), (type <= 0x10 ? rtc_regs : nullptr), rom_banks, std::get<1>(ram)));
		dma_interface = tmp.get();
		memory_interface = std::move(tmp);
	}

	else if (in_range(type, 0x19, 0x1E))
	{
		auto tmp = std::make_unique<MBC5>(MBC5(rom_ptr, std::get<0>(ram), rom_banks, std::get<1>(ram)));
		dma_interface = tmp.get();
		memory_interface = std::move(tmp);
	}

	else
		memory_interface = nullptr;
		//TODO: inform somehow that there is some unknown MBC
}
