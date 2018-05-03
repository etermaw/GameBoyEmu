#include <QFileDialog>

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    keymap_dialog(new KeyMap(this))
{
    ui->setupUi(this);
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
