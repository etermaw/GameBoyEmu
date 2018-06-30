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

void CoreThread::load_rom(const QString& path)
{
    //TODO: make it thread-safe!
    if (rom_file.isOpen())
        rom_file.close();

    rom_file.setFileName(path);

    if (rom_file.open(QIODevice::ReadOnly))
    {
        const i64 rom_size = rom_file.size();
        const u8* rom_ptr = rom_file.map(0, rom_size);

        if (!rom_ptr)
            return;

        emu_core.load_rom(rom_ptr, rom_size);
        
        const u32 ram_size = emu_core.get_ram_size();

        if (ram_size > 0)
        {
            if (emu_core.has_battery_ram())
            {
                QFileInfo ram_name(path);

                if (ram_file.isOpen())
                    ram_file.close();

                ram_file.setFileName(ram_name.baseName() + ".gbram");

                if (ram_file.open(QIODevice::ReadWrite))
                {
                    if (ram_file.size() != ram_size)
                        ram_file.resize(ram_size);

                    u8* ram_ptr = ram_file.map(0, ram_size);

                    if (ram_ptr)
                        emu_core.load_ram(ram_ptr, ram_size);
                }
            }

            else
            {
                ram_mem = std::make_unique<u8[]>(ram_size);
                emu_core.load_ram(ram_mem.get(), ram_size);
            }
        }

        if (emu_core.has_rtc())
            emu_core.load_rtc(rtc.data(), rtc.size());

        emu_core.setup_core();
    }
}

u8** CoreThread::swap_buffers(u8** buffers, u32 count)
{
    UNUSED(buffers);
    UNUSED(count);

    return dummy_buffers;
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

            input_lock.lock();

            for (u32 i = 0; i < 8; ++i)
            {
                if (event_tab[i] == 0)
                    continue;

                else if (event_tab[i] > 0)
                    emu_core.push_key(static_cast<KEYS>(i));

                else
                    emu_core.release_key(static_cast<KEYS>(i));
            }

            std::memset(event_tab, 0, sizeof(event_tab));
            input_lock.unlock();

            emu_core.run_one_frame();

            //TODO: it`s async, watch out for data races & overwriting buffer!
            emit frame_ready(frame_buffer.get()); //call me if you find better way of updating GUI thread in Qt

            //TODO: find better way of time synchronization (without busy wait!)
            const auto end = std::chrono::high_resolution_clock::now();
            const auto usecs_passed = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();

            if (usecs_passed < 16000)
                usleep(16000 - usecs_passed);
        }

        //DON`T put here condition variable, it may deadlock on exit!
    }
}
