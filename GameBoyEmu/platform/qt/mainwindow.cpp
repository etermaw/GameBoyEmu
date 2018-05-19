#include <QFileDialog>
#include <QKeyEvent>

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    keymap_dialog(new KeyMap(this))
{
    ui->setupUi(this);

    keymap_dialog->keys[0] = Qt::Key_A;
    keymap_dialog->keys[1] = Qt::Key_B;
    keymap_dialog->keys[2] = Qt::Key_Z;
    keymap_dialog->keys[3] = Qt::Key_X;

    keymap_dialog->keys[4] = Qt::Key_Up;
    keymap_dialog->keys[5] = Qt::Key_Down;
    keymap_dialog->keys[6] = Qt::Key_Right;
    keymap_dialog->keys[7] = Qt::Key_Left;

    keymap_dialog->reset_keys_to_default();

    installEventFilter(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionLoad_ROM_triggered()
{
    const QString rom_name = QFileDialog::getOpenFileName(this, tr("Select ROM to load"), ".", tr("GameBoy ROM (*.gb *.gbc)"));
}

void MainWindow::on_actionKeys_triggered()
{
    keymap_dialog->show();
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event)
{
    switch (event->type())
    {
        case QEvent::KeyPress:
        {
            QKeyEvent* key_event = static_cast<QKeyEvent*>(event);
            const auto key = key_event->key();
            //TODO: push given key to input queue for emu thread
            return true;
        }

        case QEvent::KeyRelease:
        {
            QKeyEvent* key_event = static_cast<QKeyEvent*>(event);
            const auto key = key_event->key();
            //TODO: push given key to input queue for emu thread
            return true;
        }

        default:
            return QObject::eventFilter(obj, event);
    }
}
