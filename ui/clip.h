#ifndef CLIP_H
#define CLIP_H

#include <QMainWindow>

namespace Ui {
    class Clip;
}

class Clip : public QMainWindow
{
    Q_OBJECT

public:
    explicit Clip(QWidget *parent = 0);
    ~Clip();

    void addMdiWindow(QWidget*);

public slots:
    void on_actionNew_Crystal_triggered(bool);
    void on_actionNew_Projection_triggered(bool);
    void on_actionAbout_triggered(bool);
    void on_actionAbout_Qt_triggered(bool);

private:
    Ui::Clip *ui;

};

#endif // CLIP_H
