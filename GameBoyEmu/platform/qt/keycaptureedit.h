#ifndef KEYCAPTUREEDIT_H
#define KEYCAPTUREEDIT_H

#include <QWidget>
#include <QLineEdit>
#include <QKeyEvent>


class KeyCaptureEdit : public QLineEdit
{
    Q_OBJECT

    public:
        explicit KeyCaptureEdit(QWidget* parent);

    protected:
        void keyPressEvent(QKeyEvent* event) override;

};

#endif // KEYCAPTUREEDIT_H
