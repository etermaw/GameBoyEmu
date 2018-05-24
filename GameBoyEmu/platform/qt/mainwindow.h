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

        void update_frame(u16* frame);

    protected:
        bool eventFilter(QObject* obj, QEvent* event) override;

    private:
        Ui::MainWindow *ui;
        KeyMap* keymap_dialog;
        
        CoreThread* core_thread;
};

#endif // MAINWINDOW_H
