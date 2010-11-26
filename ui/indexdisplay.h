#ifndef INDEXDISPLAY_H
#define INDEXDISPLAY_H

#include <QWidget>

namespace Ui {
    class Indexing;
}

class Crystal;

class IndexDisplay : public QWidget
{
    Q_OBJECT

public:
    explicit IndexDisplay(Crystal* _c, QWidget *parent = 0);
    ~IndexDisplay();

private:
    Ui::Indexing *ui;
    Crystal* crystal;

private slots:
    void on_startButton_clicked();
};

#endif // INDEXDISPLAY_H
