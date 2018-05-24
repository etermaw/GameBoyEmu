#include "corethread.h"

CoreThread::CoreThread(QObject* parent) : QThread(parent)
{
    frame_buffer = std::make_unique<u16[]>(144*160);
    //emu_core.set_frame_buffer(frame_buffer.get());

    std::memset(frame_buffer.get(), 0xFF, sizeof(u16) * 144 * 160);
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

            //waiter_lock.lock();
            //TODO: get input from internal input queue
            //waiter_lock.unlock();

            //emu_core.run_one_frame();

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
