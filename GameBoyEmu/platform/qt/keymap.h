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

        int keys[8];
        void reset_keys_to_default();

    private slots:
        void on_buttonBox_accepted();
        void on_buttonBox_rejected();

    private:
        Ui::KeyMap *ui;
};

#endif // KEYMAP_H
