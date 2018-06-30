#ifndef CORETHREAD_H
#define CORETHREAD_H

#include <QThread>
#include <QFile>
#include <QFileInfo>
#include <QString>

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

        void press_key(int key);
        void release_key(int key);
        void load_rom(const QString& path);

        void run() override;

    signals:
        void frame_ready(u32*);

    private:
        u8** swap_buffers(u8** buffers, u32 count);
		void dummy(bool) {}

        QFile rom_file;
        QFile ram_file;
        std::unique_ptr<u8[]> ram_mem;
        std::array<u8, 5> rtc;

        Core emu_core;

        i32 event_tab[8] = {};

        std::unique_ptr<u8[]> internal_buffer;
        u8* dummy_buffers[4];

        std::mutex input_lock;
        std::mutex waiter_lock;
        std::condition_variable core_waiter;
        std::atomic<bool> core_running{false};
        std::atomic<bool> thread_running{true};

        bool wake_up_waiter = false;

        std::unique_ptr<u32[]> frame_buffer;
};

#endif // CORETHREAD_H
