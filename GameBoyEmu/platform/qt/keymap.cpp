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

void KeyMap::reset_keys_to_default()
{
    ui->a_keycapture->set_key(keys[0]);
    ui->b_keycapture->set_key(keys[1]);
    ui->start_keycapture->set_key(keys[2]);
    ui->select_keycapture->set_key(keys[3]);

    ui->up_keycapture->set_key(keys[4]);
    ui->down_keycapture->set_key(keys[5]);
    ui->right_keycapture->set_key(keys[6]);
    ui->left_keycapture->set_key(keys[7]);
}

void KeyMap::on_buttonBox_accepted()
{
    keys[0] = ui->a_keycapture->get_key();
    keys[1] = ui->b_keycapture->get_key();
    keys[2] = ui->start_keycapture->get_key();
    keys[3] = ui->select_keycapture->get_key();

    keys[4] = ui->up_keycapture->get_key();
    keys[5] = ui->down_keycapture->get_key();
    keys[6] = ui->right_keycapture->get_key();
    keys[7] = ui->left_keycapture->get_key();

    close();
}

void KeyMap::on_buttonBox_rejected()
{
    reset_keys_to_default();
    close();
}
