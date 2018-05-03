#include "core/core.h"
#include "test.h"

int main(int argc, char *argv[])
{
    std::string file_name;

	auto ram_saver = [&](const u8* data, u32 size)
	{
		std::ofstream to_save(file_name + "_ram", std::ios::trunc | std::ios::binary);
		to_save.write(reinterpret_cast<const char*>(data), size * sizeof(u8));
	};

	auto rtc_saver = [&](std::chrono::seconds epoch, const u8* data, u32 size)
	{
		std::ofstream to_save(file_name + "_rtc", std::ios::trunc | std::ios::binary);
		to_save << epoch.count();
		to_save.write(reinterpret_cast<const char*>(data), size * sizeof(u8));
	};

	Core emu_core;

    Tester test;
    AudioTEST audio_stub;

	external_callbacks endpoints;

	endpoints.save_ram = function<void(const u8*, u32)>(ram_saver);
	endpoints.save_rtc = function<void(std::chrono::seconds, const u8*, u32)>(rtc_saver);
	endpoints.audio_control = make_function(&AudioTEST::dummy, &audio_stub);
	endpoints.swap_sample_buffer = make_function(&AudioTEST::swap_buffers, &audio_stub);
	endpoints.draw_frame = make_function(&Tester::render_stub, &test);

	emu_core.attach_callbacks(endpoints);

	std::ifstream rom(argv[1], std::ios::binary);

	if (rom.is_open())
	{
		std::ifstream ram(file_name + "_ram", std::ios::binary), rtc(file_name + "_rtc", std::ios::binary);
		emu_core.load_cartrige(rom, ram, rtc);

        while (test.is_running())
        {
            test.input_stub(emu_core);
            emu_core.run_one_frame();
        }
	}	

	return 0;
}
