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
    void slotSetQRange();
    void slotLoadParams();
    void slotSetProjectionDirection();
    void slotUpdateRenderHints();
private:
    Ui::StereoCfg *ui;
    StereoProjector* projector;
};

#endif // STEREOCFG_H
