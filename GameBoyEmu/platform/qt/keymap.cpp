#include <QRegularExpression>

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

void KeyMap::check_highlight_field(QKeySequenceEdit* edit)
{
    static const QRegularExpression field_regex("[[:alnum:]]+\\+[[:alnum:]]+"); //TODO: why making it class field introduces segfaults on exit?
    const QString keys = edit->keySequence().toString();

    if (keys.contains(field_regex))
        edit->setStyleSheet("border: 1px solid red; border-radius: 2px;");

    else
        edit->setStyleSheet("");
}

void KeyMap::on_a_key_field_editingFinished()
{
    check_highlight_field(ui->a_key_field);
}

void KeyMap::on_b_key_field_editingFinished()
{
    check_highlight_field(ui->b_key_field);
}

void KeyMap::on_start_key_field_editingFinished()
{
    check_highlight_field(ui->start_key_field);
}

void KeyMap::on_select_key_field_editingFinished()
{
    check_highlight_field(ui->select_key_field);
}

void KeyMap::on_up_key_field_editingFinished()
{
    check_highlight_field(ui->up_key_field);
}

void KeyMap::on_down_key_field_editingFinished()
{
    check_highlight_field(ui->down_key_field);
}

void KeyMap::on_left_key_field_editingFinished()
{
    check_highlight_field(ui->left_key_field);
}

void KeyMap::on_right_key_field_editingFinished()
{
    check_highlight_field(ui->right_key_field);
}
