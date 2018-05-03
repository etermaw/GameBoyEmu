#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "keymap.h"

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

    private:
        Ui::MainWindow *ui;
        KeyMap* keymap_dialog;
};

#endif // MAINWINDOW_H
