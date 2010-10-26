#include "stereocfg.h"
#include "ui_stereocfg.h"

#include <QGraphicsView>


StereoCfg::StereoCfg(StereoProjector* p, QWidget *parent) :
    QWidget(parent),    
    ui(new Ui::StereoCfg),
    projector(p)
{
  ui->setupUi(this);
  connect(p, SIGNAL(destroyed()), this, SLOT(deleteLater()));
  connect(ui->detTextSize, SIGNAL(valueChanged(double)), p, SLOT(setTextSize(double)));
  connect(ui->detSpotSize, SIGNAL(valueChanged(double)), p, SLOT(setSpotSize(double)));
  connect(ui->maxRefLabel, SIGNAL(valueChanged(int)), p, SLOT(setMaxHklSqSum(int)));
  connect(ui->detMinQ, SIGNAL(valueChanged(double)), this, SLOT(slotSetQRange()));
  connect(ui->detMaxQ, SIGNAL(valueChanged(double)), this, SLOT(slotSetQRange()));
  connect(p, SIGNAL(projectionParamsChanged()), this, SLOT(slotLoadParams()));

  connect(ui->Xp, SIGNAL(clicked()), this, SLOT(slotSetProjectionDirection()));
  connect(ui->Xm, SIGNAL(clicked()), this, SLOT(slotSetProjectionDirection()));
  connect(ui->Yp, SIGNAL(clicked()), this, SLOT(slotSetProjectionDirection()));
  connect(ui->Ym, SIGNAL(clicked()), this, SLOT(slotSetProjectionDirection()));
  connect(ui->Zp, SIGNAL(clicked()), this, SLOT(slotSetProjectionDirection()));
  connect(ui->Zm, SIGNAL(clicked()), this, SLOT(slotSetProjectionDirection()));
  connect(ui->invertProjection, SIGNAL(toggled(bool)), SLOT(slotSetProjectionDirection()));

  connect(ui->renderAntialias, SIGNAL(toggled(bool)), this, SLOT(slotUpdateRenderHints()));
  connect(ui->renderAntialiasText, SIGNAL(toggled(bool)), this, SLOT(slotUpdateRenderHints()));

  slotLoadParams();
}

StereoCfg::~StereoCfg() {
  delete ui;
}

void StereoCfg::slotLoadParams() {
  ui->detTextSize->setValue(projector->getTextSize());
  ui->detSpotSize->setValue(projector->getSpotSize());
  ui->maxRefLabel->setValue(projector->getMaxHklSqSum());
  ui->detMinQ->setValue(projector->Qmin());
  ui->detMaxQ->setValue(projector->Qmax());

  Mat3D M = projector->getDetOrientation();
  Vec3D v = M.transposed()*Vec3D(1,0,0);
  int i = (v*Vec3D(1,2,3))*M.det();

  if (i==1) {
    ui->Xm->setChecked(true);
  } else if (i==-1) {
    ui->Xp->setChecked(true);
  } else if (i==2) {
    ui->Ym->setChecked(true);
  } else if (i==-2) {
    ui->Yp->setChecked(true);
  } else if (i==3) {
    ui->Zm->setChecked(true);
  } else if (i==-3) {
    ui->Zp->setChecked(true);
  }
  ui->invertProjection->setChecked(M.det()<0);

  QList<QGraphicsView*> l = projector->getScene()->views();
  if (l.size()) {
    ui->renderAntialias->setChecked(l.at(0)->renderHints()&QPainter::Antialiasing);
    ui->renderAntialiasText->setChecked(l.at(0)->renderHints()&QPainter::TextAntialiasing);
  }

}

void StereoCfg::slotSetQRange() {
  projector->setWavevectors(ui->detMinQ->value(), ui->detMaxQ->value());
  ui->QDispA->setValue(2.0*M_PI/ui->detMaxQ->value());
  ui->QDispKEV->setValue(0.5*M_1_PI*12.398*ui->detMaxQ->value());
}

void StereoCfg::slotSetProjectionDirection() {
  Mat3D M;
  if (ui->Xp->isChecked()) {
    M = Mat3D(-1, 0, 0, 0,-1, 0, 0, 0, 1);
  } else if (ui->Xm->isChecked()) {
    M = Mat3D( 1, 0, 0, 0, 1, 0, 0, 0, 1);
  } else if (ui->Yp->isChecked()) {
    M = Mat3D( 0,-1, 0, 1, 0, 0, 0, 0, 1);
  } else if (ui->Ym->isChecked()) {
    M = Mat3D( 0, 1, 0,-1, 0, 0, 0, 0, 1);
  } else if (ui->Zp->isChecked()) {
    M = Mat3D( 0, 0,-1,-1, 0, 0, 0, 1, 0);
  } else if (ui->Zm->isChecked()) {
    M = Mat3D( 0, 0, 1, 1, 0, 0, 0, 1, 0);
  }
  if (ui->invertProjection->isChecked())
    M = Mat3D(-1, 0, 0, 0, 1, 0, 0, 0, 1) * M;
  projector->setDetOrientation(M);
}

void StereoCfg::slotUpdateRenderHints() {
  foreach(QGraphicsView* view, projector->getScene()->views()) {
    view->setRenderHint(QPainter::Antialiasing, ui->renderAntialias->isChecked());
    view->setRenderHint(QPainter::TextAntialiasing, ui->renderAntialiasText->isChecked());
  }
}
