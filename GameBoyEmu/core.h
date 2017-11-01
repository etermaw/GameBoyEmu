#pragma once
#include "stdafx.h"
#include "cpu.h"
#include "mmu.h"
#include "ram.h"
#include "cartrige.h"
#include "interrupts.h"
#include "gpu.h"
#include "timer.h"
#include "joypad.h"
#include "apu.h"
#include "debugger.h"

struct external_callbacks
{
	function<void(const u8*, u32)> save_ram;
	function<void(std::chrono::seconds, const u8*, u32)> save_rtc;

	function<void(const u32*)> draw_frame;
	function<bool(Joypad&)> update_input;

	function<u8**(u8**, u32)> swap_sample_buffer;
	function<void(bool)> audio_control;
	function<void()> audio_synchronize; //TODO: reserved for future use
};

struct TestReader final : public IMemory
{
	u8 a;

public:
	TestReader() : a() {}

	u8 read_byte(u16 adress, u32 unused) override
	{
		return 0;
		//return 0xFF;
	}

	void write_byte(u16 adress, u8 val, u32 unused) override
	{
		if (adress == 0xFF01)
			a = val;

		if (adress == 0xFF02 && val == 0x81)
			printf("%c", a);
	}
};

struct SpeedSwitch : public IMemory
{
	bool double_speed = false;
	bool switch_speed = false;

	u8 read_byte(u16 adress, u32 unused) override
	{
		if (adress == 0xFF4D)
		{
			auto ret = change_bit(0xFF, double_speed, 7);
			return change_bit(ret, switch_speed, 0);
		}

		else
			return 0xFF;
	}

	void write_byte(u16 adress, u8 val, u32 key) override
	{
		if (adress == 0xFF4D)
		{
			if (key == 0xFFFFFFFF && val == 0xFF)
			{
				switch_speed = false;
				double_speed = !double_speed;
			}

			else
				switch_speed = check_bit(val, 0);
		}

		//else ignore
	}

	void serialize(std::ostream& stream)
	{
		stream << switch_speed << double_speed;
	}

	void deserialize(std::istream& stream)
	{
		stream >> switch_speed >> double_speed;
	}
};

class Core
{
	private:
		Interrupts ints;
		MMU mmu;

		CPU cpu;
		Gpu gpu;
		APU apu;

		Timer timer;
		Cartrige cart;
		Ram ram;
		TestReader tr; //testing
		Joypad joypad;
		SpeedSwitch speed;

		Debugger debugger;

		function<void(const u32*)> draw_frame_callback;
		function<bool(Joypad&)> pump_input_callback;

	public:
		Core();
		//~Core();

		//void reset();
		bool load_cartrige(std::ifstream& rom_file, std::ifstream& ram_file, std::ifstream& rtc_file);
		void load_state(std::istream& load_stream);
		void save_state(std::ostream& save_stream);
		void run();

		void attach_callbacks(const external_callbacks& endpoints);
};

