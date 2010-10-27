#ifndef LAUEPLANECFG_H
#define LAUEPLANECFG_H

#include <QWidget>
#include <core/laueplaneprojector.h>

namespace Ui {
  class LauePlaneCfg;
}

class LauePlaneCfg : public QWidget
{
  Q_OBJECT

public:
  explicit LauePlaneCfg(LauePlaneProjector*, QWidget *parent = 0);
  ~LauePlaneCfg();
public slots:
  void slotSetQRange();
  void slotLoadParams();
  void slotUpdateRenderHints();
private:
  Ui::LauePlaneCfg *ui;
  LauePlaneProjector* projector;
};

#endif // LAUEPLANECFG_H
