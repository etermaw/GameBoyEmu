#ifdef ENABLE_TEST_DISPLAY
#include "platform/sdl/gui.h"
#include "platform/sdl/renderer.h"
#endif

#include "platform/linux/test.h"
#include "core/core.h"

int main(int argc, char *argv[])
{
	std::string file_name;

	auto rtc_saver = [&](std::chrono::seconds epoch, const u8* data, u32 size)
	{
		std::ofstream to_save(file_name + "_rtc", std::ios::trunc | std::ios::binary);
		to_save << epoch.count();
		to_save.write(reinterpret_cast<const char*>(data), size * sizeof(u8));
	};

	Core emu_core;
	Tester auto_tester;
#ifdef ENABLE_TEST_DISPLAY
	GuiSDL gui(3*160, 3*144, "GBE");
	RendererSDL renderer(gui.get_display());

	auto_tester.attach_renderer(make_function(&RendererSDL::draw_frame, &renderer));
#endif

	external_callbacks endpoints;

	endpoints.audio_control = make_function(&Tester::dummy, &auto_tester);
	endpoints.swap_sample_buffer = make_function(&Tester::swap_buffers, &auto_tester);

	emu_core.attach_callbacks(endpoints);

	std::ifstream rom(argv[1], std::ios::binary);
	std::unique_ptr<u8[]> rom_mem;
	std::unique_ptr<u8[]> ram_mem;
	u8 rtc_mem[5] = {};

	if (rom.is_open())
	{
		//load rom
		rom.seekg(0, std::ios_base::end);
		const auto rom_size = rom.tellg();
		rom.seekg(0, std::ios_base::beg);

		rom_mem = std::make_unique<u8[]>(rom_size);
		rom.read(reinterpret_cast<char*>(rom_mem.get()), rom_size);

		//attach rom
		emu_core.load_rom(rom_mem.get(), rom_size);

		//if cartrige has ram, create it
		const u32 ram_size = emu_core.get_ram_size();

		if (ram_size > 0)
		{
			ram_mem = std::make_unique<u8[]>(ram_size);

			if (emu_core.has_battery_ram())
			{
				std::ifstream ram(file_name + "_ram", std::ios::binary);
				ram.read(reinterpret_cast<char*>(ram_mem.get()), ram_size);
			}

			emu_core.load_ram(ram_mem.get(), ram_size);
		}

		if (emu_core.has_rtc())
		{
			//TODO: implement RTC (again!)
			emu_core.load_rtc(rtc_mem, 5);

			//std::ifstream rtc(file_name + "_rtc", std::ios::binary);
		}

		emu_core.setup_core();
	}

	emu_core.set_frame_buffer(auto_tester.draw_frame());

	while (auto_tester.is_running())
	{
		auto_tester.pump_input(emu_core);
		emu_core.run_one_frame();
		emu_core.set_frame_buffer(auto_tester.draw_frame());
	}

	return 0;
}
