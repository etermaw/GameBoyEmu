#pragma once
#include "stdafx.h"
#include "IMemory.h"
#include "mbc.h"

class Cartrige
{
	private:
		std::string file_name;

		std::unique_ptr<u8[]> rom;
		std::unique_ptr<u8[]> ram;
		std::unique_ptr<IMemory> memory_interface;

		size_t ram_size;
		u8 rtc_regs[5];
		bool battery_ram;

		void load_ram();
		void dispatch();

	public:
		~Cartrige();

		void load_cartrige(const std::string& name);
		IMemory* get_memory_interface() const;
};
