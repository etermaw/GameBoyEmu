#include "keycaptureedit.h"

KeyCaptureEdit::KeyCaptureEdit(QWidget* parent) :
    QLineEdit(parent)
{
    QLineEdit::setReadOnly(true);
    QLineEdit::setPlaceholderText("Press key");
}

void KeyCaptureEdit::keyPressEvent(QKeyEvent* event)
{
    int key = event->key();

    if (key != Qt::Key_Shift && key != Qt::Key_Control && key != Qt::Key_Alt && key != Qt::Key_Meta)
        QLineEdit::setText(QKeySequence(key).toString());
}
