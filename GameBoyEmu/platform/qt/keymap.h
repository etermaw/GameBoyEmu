#ifndef KEYMAP_H
#define KEYMAP_H

#include <QKeySequenceEdit>
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

    private slots:
        void on_a_key_field_editingFinished();
        void on_b_key_field_editingFinished();
        void on_start_key_field_editingFinished();
        void on_select_key_field_editingFinished();
        void on_up_key_field_editingFinished();
        void on_down_key_field_editingFinished();
        void on_left_key_field_editingFinished();
        void on_right_key_field_editingFinished();

    private:
        Ui::KeyMap *ui;

        void check_highlight_field(QKeySequenceEdit* edit);
};

#endif // KEYMAP_H
