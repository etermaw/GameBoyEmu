#ifndef CORETHREAD_H
#define CORETHREAD_H

#include <QObject>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include "core/core.h"

class CoreThread : public QObject
{
    Q_OBJECT
    public:
        explicit CoreThread(QObject *parent = nullptr);
        ~CoreThread();

        void halt_emulation();
        void start_emulation();

    signals:

    public slots:

    private:
        std::thread core_thread;
        std::mutex core_lock;
        std::condition_variable core_waiter;
        std::atomic<bool> core_running;
        std::atomic<bool> thread_running;

        Core emu_core;

        void core_main();
};

#endif // CORETHREAD_H
