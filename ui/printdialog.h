#ifndef PRINTDIALOG_H
#define PRINTDIALOG_H

#include <QMainWindow>

namespace Ui {
    class PrintDialog;
}

class PrintDialog : public QMainWindow
{
    Q_OBJECT

public:
    explicit PrintDialog(QWidget *parent = 0);
    ~PrintDialog();

private:
    Ui::PrintDialog *ui;

private slots:
    void on_actionInsert_Cell_Table_triggered();
};

#endif // PRINTDIALOG_H
