#include "mouseinfodisplay.h"
#include "ui_mouseinfodisplay.h"

MouseInfoDisplay::MouseInfoDisplay(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MouseInfoDisplay)
{
    ui->setupUi(this);
}

MouseInfoDisplay::~MouseInfoDisplay()
{
    delete ui;
}



void MouseInfoDisplay::showMouseInfo(MousePositionInfo info) {
  class Macro {
  public:
    Macro(QLineEdit* line, double val, bool enabled) {
      if (enabled) {
        line->setText(QString::number(val, 'f', 3));
      } else {
        line->setText("");
      }
    }
    Macro(QLineEdit* line, QString val, bool enabled) {
      if (enabled) {
        line->setText(val);
      } else {
        line->setText("");
      }
    }
  };

  Macro(ui->det_x, info.projectorPos.x(), info.valid);
  Macro(ui->det_y, info.projectorPos.y(), info.valid);

  Macro(ui->norm_x, info.normal.x(), info.valid);
  Macro(ui->norm_y, info.normal.y(), info.valid);
  Macro(ui->norm_z, info.normal.z(), info.valid);

  Macro(ui->sr_x, info.scattered.x(), info.valid && info.scatteredOk);
  Macro(ui->sr_y, info.scattered.y(), info.valid && info.scatteredOk);
  Macro(ui->sr_z, info.scattered.z(), info.valid && info.scatteredOk);


  if (info.nearestOk) {
    const Reflection& r = info.nearestReflection;
    ui->reflex->setText(r.toText());
    Macro(ui->infoD, r.d, true);
    Macro(ui->infoQ, r.Qscatter, r.normal.x()>1e-6);
    Macro(ui->info2T, 180.0-180.0*M_1_PI*acos(r.scatteredRay.x()), r.normal.x()>1e-6);
    Vec3D n = r.normal;
    QList<QPair<QLineEdit*, double> > l;
    l << qMakePair(ui->axX ,  n(0));
    l << qMakePair(ui->axMX, -n(0));
    l << qMakePair(ui->axY ,  n(1));
    l << qMakePair(ui->axMY, -n(1));
    l << qMakePair(ui->axZ ,  n(2));
    l << qMakePair(ui->axMZ, -n(2));
    QPair<QLineEdit*, double> pair;
    foreach (pair, l) pair.first->setText(QString::number(180*M_1_PI*acos(qBound(-1.0, pair.second, 1.0)), 'f', 2));
    QString diffOrders;
    foreach (int i, r.orders) {
      if ((2.0*info.detQMin<=i*r.Qscatter) && (i*r.Qscatter<=2.0*info.detQMax)) {
        if (!diffOrders.isEmpty()) diffOrders += ", ";
        diffOrders += QString::number(i);
      }
    }
    ui->diffOrders->setText(diffOrders);
  /*                h, k, l=hkl

                self.diffOrders.setText('')
                for r in c.getReflectionList():
                    if r.h==h and r.k==k and r.l==l:
                        t=''
                        for n in r.orders:
                            if n>=r.lowestDiffOrder and n<=r.highestDiffOrder:
                                t+='%i, '%n
                        if len(t)>0:
                            self.diffOrders.setText(t[:-2])
                        break
*/





  }
}
