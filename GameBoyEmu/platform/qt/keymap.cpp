#include "keymap.h"
#include "ui_keymap.h"

KeyMap::KeyMap(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::KeyMap)
{
    ui->setupUi(this);
}

KeyMap::~KeyMap()
{
    delete ui;
}
