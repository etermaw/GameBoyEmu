#include "corethread.h"

static constexpr u32 BUFFER_SIZE = 1 << 15;

CoreThread::CoreThread(QObject* parent) : QThread(parent)
{
    frame_buffer = std::make_unique<u32[]>(144*160);
    emu_core.set_frame_buffer(frame_buffer.get());

    std::memset(frame_buffer.get(), 0xFF, sizeof(u32) * 144 * 160);

    internal_buffer = std::make_unique<u8[]>(BUFFER_SIZE * 4);

	for (u32 i = 0; i < 4; ++i)
		dummy_buffers[i] = &internal_buffer[BUFFER_SIZE * i];


    external_callbacks endpoints;

	endpoints.save_ram = make_function(&CoreThread::save_ram, this);
	endpoints.save_rtc = make_function(&CoreThread::save_rtc, this);
	endpoints.audio_control = make_function(&CoreThread::dummy, this);
	endpoints.swap_sample_buffer = make_function(&CoreThread::swap_buffers, this);

	emu_core.attach_callbacks(endpoints);
}

CoreThread::~CoreThread()
{
    stop();
}

void CoreThread::halt_emulation()
{
    core_running.store(false);
}

void CoreThread::start_emulation()
{
    core_running.store(true);

    waiter_lock.lock();
    wake_up_waiter = true;
    waiter_lock.unlock();

    core_waiter.notify_one();
}

void CoreThread::stop()
{
    thread_running.store(false);
    core_running.store(false);

    waiter_lock.lock();
    wake_up_waiter = true;
    waiter_lock.unlock();

    core_waiter.notify_all();
}

void CoreThread::press_key(int key)
{
    std::unique_lock<std::mutex> lock(input_lock);
    event_tab[key] = std::max(event_tab[key] + 1, 1);
}

void CoreThread::release_key(int key)
{
    std::unique_lock<std::mutex> lock(input_lock);
    event_tab[key] = std::min(event_tab[key] - 1, -1);
}

void CoreThread::load_rom(const std::string& path)
{
    file_name = path;
    rom.open(path, std::ios::binary);

    emu_core.load_cartrige(rom, ram, rtc);
}

u8** CoreThread::swap_buffers(u8** buffers, u32 count)
{
    UNUSED(buffers);
    UNUSED(count);

    return dummy_buffers;
}

void CoreThread::save_ram(const u8* data, u32 size)
{
    std::ofstream to_save(file_name + "_ram", std::ios::trunc | std::ios::binary);
	to_save.write(reinterpret_cast<const char*>(data), size * sizeof(u8));
}

void CoreThread::save_rtc(std::chrono::seconds epoch, const u8* data, u32 size)
{
    std::ofstream to_save(file_name + "_rtc", std::ios::trunc | std::ios::binary);
	to_save << epoch.count();
	to_save.write(reinterpret_cast<const char*>(data), size * sizeof(u8));
}

void CoreThread::run()
{
    while (thread_running)
    {
        //if core is not running, sleep
        std::unique_lock<std::mutex> lock(waiter_lock);

        core_waiter.wait(lock, [this](){ return wake_up_waiter; });
        wake_up_waiter = false;

        lock.unlock();

        while (core_running)
        {
            const auto begin = std::chrono::high_resolution_clock::now();

            //input_lock.lock();
            //TODO: get input from internal input queue
            //input_lock.unlock();

            emu_core.run_one_frame();

            //TODO: it`s async, watch out for data races & overwriting buffer!
            emit frame_ready(frame_buffer.get()); //call me if you find better way of updating GUI thread in Qt

            const auto end = std::chrono::high_resolution_clock::now();
            const auto usecs_passed = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();

            if (usecs_passed < 16000)
                usleep(16000 - usecs_passed);
        }

        //DON`T put here condition variable, it may deadlock on exit!
    }
}
