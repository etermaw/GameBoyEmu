#include "keycaptureedit.h"

KeyCaptureEdit::KeyCaptureEdit(QWidget* parent) :
    QLineEdit(parent)
{
    QLineEdit::setReadOnly(true);
    QLineEdit::setPlaceholderText("Press key");
}

void KeyCaptureEdit::set_key(int key)
{
    if (key != Qt::Key_Shift && key != Qt::Key_Control && key != Qt::Key_Alt && key != Qt::Key_Meta)
    {
        QLineEdit::setText(QKeySequence(key).toString());
        current_key = key;
    }
}

int KeyCaptureEdit::get_key() const
{
    return current_key;
}

void KeyCaptureEdit::keyPressEvent(QKeyEvent* event)
{
    set_key(event->key());
}
