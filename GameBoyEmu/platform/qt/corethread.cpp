#include "corethread.h"

CoreThread::CoreThread(QObject *parent) : QObject(parent)
{
    thread_running.store(true);
    core_running.store(false);

    core_thread = std::thread(&CoreThread::core_main, this);
}

CoreThread::~CoreThread()
{
    thread_running.store(false);
    core_running.store(false);
    core_waiter.notify_all();

    if (core_thread.joinable())
        core_thread.join();
}

void CoreThread::halt_emulation()
{
    core_running.store(false);
}

void CoreThread::start_emulation()
{
    core_running.store(true);
    core_waiter.notify_all();
}

void CoreThread::core_main()
{
    //general paranoic rule: anything shared MUST be either atomic or protected by mutex

    while (thread_running)
    {
        std::unique_lock<std::mutex> lock(core_lock);
        core_waiter.wait(lock);
        lock.unlock();

        while (core_running)
        {
            //TODO: get input from internal input queue (access MUST be mutexed)
            emu_core.run_one_frame();
            //TODO: push out frame (also MUST be mutexed)
        }
    }
}
