#ifndef CORETHREAD_H
#define CORETHREAD_H

#include <QThread>

#include "core/core.h"

class CoreThread : public QThread
{
    Q_OBJECT
    public:
        explicit CoreThread(QObject *parent = nullptr);
        ~CoreThread();

        void halt_emulation();
        void start_emulation();
        void stop();

        void push_into_event_queue(int key);
        void load_rom(const std::string& path);

        void run() override;

    signals:
        void frame_ready(u32*);

    private:
        u8** swap_buffers(u8** buffers, u32 count);
		void dummy(bool) {}

        void save_ram(const u8* data, u32 size);
        void save_rtc(std::chrono::seconds epoch, const u8* data, u32 size);

        //Core emu_core;

        std::unique_ptr<u8[]> internal_buffer;
        u8* dummy_buffers[4];

        std::ifstream rom, ram, rtc;
        std::string file_name;

        std::mutex waiter_lock;
        std::condition_variable core_waiter;
        std::atomic<bool> core_running{false};
        std::atomic<bool> thread_running{true};

        bool wake_up_waiter = false;

        std::unique_ptr<u32[]> frame_buffer;
};

#endif // CORETHREAD_H
