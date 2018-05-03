#ifndef KEYMAP_H
#define KEYMAP_H

#include <QDialog>

namespace Ui {
    class KeyMap;
}

class KeyMap : public QDialog
{
    Q_OBJECT

    public:
        explicit KeyMap(QWidget *parent = 0);
        ~KeyMap();

    private:
        Ui::KeyMap *ui;
};

#endif // KEYMAP_H
