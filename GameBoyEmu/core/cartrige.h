#pragma once

#include "IMemory.h"
#include "mbc.h"

class Cartrige
{
	private:
		function<void(const u8*, u32)> save_ram_callback;
		function<void(std::chrono::seconds, const u8*, u32)> save_rtc_callback;

		//[ptr, size of memory]
		std::pair<const u8*, u32> rom;
		std::pair<u8*, u32> ram;
		std::pair<u8*, u32> rtc2;

		std::unique_ptr<IMemory> memory_interface;
		IDmaMemory* dma_interface;

		u8 rtc_regs[5] = {};

		void load_rtc(std::ifstream& file);
		void dispatch();

	public:
		//TODO: add function to validate ROM
		void attach_rom(const u8* rom_ptr, u32 size);
		void attach_ram(u8* ram_ptr, u32 size);
		void attach_rtc(u8* rtc_ptr, u32 size);

		bool has_battery_ram() const;
		u32 get_declared_ram_size() const;

		bool has_rtc() const;

		void setup();
		
		IMemory* get_memory_controller() const;
		IDmaMemory* get_dma_controller() const;
		bool is_cgb_ready() const;

        std::string get_name() const;
		
		void reset();
		void serialize(std::ostream& stream);
		void deserialize(std::istream& stream);
};
