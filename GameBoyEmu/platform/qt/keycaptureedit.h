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

        void set_key(int key);
        int get_key() const;

    protected:
        void keyPressEvent(QKeyEvent* event) override;

    private:
        int current_key;
};

#endif // KEYCAPTUREEDIT_H
