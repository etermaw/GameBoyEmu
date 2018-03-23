#pragma once
#include "stdafx.h"
#include "IMemory.h"
#include "mbc.h"

class Cartrige
{
	private:
		function<void(const u8*, u32)> save_ram_callback;
		function<void(std::chrono::seconds, const u8*, u32)> save_rtc_callback;

		std::unique_ptr<u8[]> rom;
		std::unique_ptr<u8[]> ram;
		std::unique_ptr<IMemory> memory_interface;
		IDmaMemory* dma_interface;

		size_t ram_size;
		u8 rtc_regs[5] = {};
		bool battery_ram;

		void load_or_create_ram(std::ifstream& file);
		void load_rtc(std::ifstream& file);
		void dispatch();

	public:
		~Cartrige();

		bool load_cartrige(std::ifstream& cart, std::ifstream& ram, std::ifstream& rtc);
		void attach_endpoints(function<void(const u8*, u32)> ram_save, function<void(std::chrono::seconds, const u8*, u32)> rtc_save);
		
		IMemory* get_memory_controller() const;
		IDmaMemory* get_dma_controller() const;
		bool is_cgb_ready() const;

        std::string get_name() const;

		void serialize(std::ostream& stream);
		void deserialize(std::istream& stream);
};
