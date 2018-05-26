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

        void push_into_event_queue(int key);

        void run() override;

        void stop();

    signals:
        void frame_ready(u16*);

    private:
        //Core emu_core;

        std::mutex waiter_lock;
        std::condition_variable core_waiter;
        std::atomic<bool> core_running{false};
        std::atomic<bool> thread_running{true};

        bool wake_up_waiter = false;

        std::unique_ptr<u16[]> frame_buffer;
};

#endif // CORETHREAD_H
