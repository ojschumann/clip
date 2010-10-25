#ifndef STEREOCFG_H
#define STEREOCFG_H

#include <QWidget>
#include <core/stereoprojector.h>


namespace Ui {
    class StereoCfg;
}

class StereoCfg : public QWidget
{
    Q_OBJECT

public:
    explicit StereoCfg(StereoProjector*, QWidget *parent = 0);
    ~StereoCfg();
  public slots:
    void slotProjectorClosed();
private:
    Ui::StereoCfg *ui;
};

#endif // STEREOCFG_H
