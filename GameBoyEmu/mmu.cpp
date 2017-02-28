#include "mmu.h"

IMemory* MMU::find_chunk(u16 adress)
{
	//search using binary search for memory chunk of coresponding range 
	auto it = std::lower_bound(std::begin(chunks), std::end(chunks), adress,
		[](auto element, auto adr) 
		{
			return (element.first < adr); 
		});

	assert(it != std::end(chunks));

	return it->second;
}

void MMU::register_chunk(u16 start, u16 end, IMemory* handler)
{
	assert(start <= end);

	chunks.push_back({ end, handler });
}

u8 MMU::read_byte(u16 adress)
{
	return find_chunk(adress)->read_byte(adress);
}

void MMU::write_byte(u16 adress, u8 value)
{
	find_chunk(adress)->write_byte(adress, value);
}