#include <QFileDialog>
#include <QKeyEvent>
#include <QDir>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "renderwidget.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    keymap_dialog(new KeyMap(this)),
    core_thread(new CoreThread(this))
{
    ui->setupUi(this);

    //setup key binding ui
    keymap_dialog->keys[0] = Qt::Key_A;
    keymap_dialog->keys[1] = Qt::Key_S;
    keymap_dialog->keys[2] = Qt::Key_Z;
    keymap_dialog->keys[3] = Qt::Key_X;

    keymap_dialog->keys[4] = Qt::Key_Up;
    keymap_dialog->keys[5] = Qt::Key_Down;
    keymap_dialog->keys[6] = Qt::Key_Right;
    keymap_dialog->keys[7] = Qt::Key_Left;

    keymap_dialog->reset_keys_to_default();

    //set up keymaps
    key_map[Qt::Key_Right] = KEYS::K_RIGHT;
    key_map[Qt::Key_Left] = KEYS::K_LEFT;
    key_map[Qt::Key_Up] = KEYS::K_UP;
    key_map[Qt::Key_Down] = KEYS::K_DOWN;

    key_map[Qt::Key_A] = KEYS::K_A;
    key_map[Qt::Key_S] = KEYS::K_B;
    key_map[Qt::Key_Z] = KEYS::K_SELECT;
    key_map[Qt::Key_X] = KEYS::K_START;

    //setup signal for key change
    connect(keymap_dialog, &KeyMap::accepted, this, &MainWindow::update_keys);

    //set up input filter
    installEventFilter(this);

    //set up worker thread
    core_thread->setObjectName("emulator_core");
    connect(core_thread, &CoreThread::finished, this, &QObject::deleteLater);
    connect(core_thread, &CoreThread::frame_ready, this, &MainWindow::update_frame, Qt::QueuedConnection);
    core_thread->start();
}

MainWindow::~MainWindow()
{
    core_thread->stop();
    core_thread->wait();
    delete ui;
}

void MainWindow::on_actionLoad_ROM_triggered()
{
    const QString rom_path = QFileDialog::getOpenFileName(this, tr("Select ROM to load"), ".", tr("GameBoy ROM (*.gb *.gbc)"));
    const auto path_splitted = rom_path.split(QDir::separator());

    setWindowTitle("Emu: " + path_splitted.last());

    core_thread->load_rom(rom_path);
    core_thread->start_emulation();
}

void MainWindow::on_actionKeys_triggered()
{
    keymap_dialog->show();
}

void MainWindow::update_keys()
{
    key_map.clear();

    key_map[keymap_dialog->keys[0]] = KEYS::K_A;
    key_map[keymap_dialog->keys[1]] = KEYS::K_B;
    key_map[keymap_dialog->keys[2]] = KEYS::K_SELECT;
    key_map[keymap_dialog->keys[3]] = KEYS::K_START;

    key_map[keymap_dialog->keys[4]] = KEYS::K_UP;
    key_map[keymap_dialog->keys[5]] = KEYS::K_DOWN;
    key_map[keymap_dialog->keys[6]] = KEYS::K_RIGHT;
    key_map[keymap_dialog->keys[7]] = KEYS::K_LEFT;
}

void MainWindow::update_frame(u32* frame)
{
    ui->openGLWidget->update_frame(frame);
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event)
{
    switch (event->type())
    {
        case QEvent::KeyPress:
        {
            QKeyEvent* key_event = static_cast<QKeyEvent*>(event);
            const auto key = key_event->key();
            const auto it = key_map.find(key);

            if (it != key_map.end())
                core_thread->press_key(it->second);

            return true;
        }

        case QEvent::KeyRelease:
        {
            QKeyEvent* key_event = static_cast<QKeyEvent*>(event);
            const auto key = key_event->key();
            const auto it = key_map.find(key);

            if (it != key_map.end())
                core_thread->release_key(it->second);

            return true;
        }

        default:
            return QObject::eventFilter(obj, event);
    }
}
