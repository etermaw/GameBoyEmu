#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "keymap.h"
#include "corethread.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent = 0);
        ~MainWindow();

    private slots:
        void on_actionLoad_ROM_triggered();
        void on_actionKeys_triggered();

        void update_keys();
        void update_frame(u32* frame);

    protected:
        bool eventFilter(QObject* obj, QEvent* event) override;

    private:
        Ui::MainWindow *ui;
        KeyMap* keymap_dialog;

        std::unordered_map<int, KEYS> key_map;
        CoreThread* core_thread;
};

#endif // MAINWINDOW_H
