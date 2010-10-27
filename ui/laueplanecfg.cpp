#include "laueplanecfg.h"
#include "ui_laueplanecfg.h"

#include <QGraphicsView>
#include <QPair>

LauePlaneCfg::LauePlaneCfg(LauePlaneProjector* p, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LauePlaneCfg),
    projector(p)
{
  ui->setupUi(this);
  connect(p, SIGNAL(destroyed()), this, SLOT(deleteLater()));

  connect(ui->maxRefLabel, SIGNAL(valueChanged(int)), p, SLOT(setMaxHklSqSum(int)));
  connect(ui->detMinQ, SIGNAL(valueChanged(double)), this, SLOT(slotSetQRange()));
  connect(ui->detMaxQ, SIGNAL(valueChanged(double)), this, SLOT(slotSetQRange()));
  connect(p, SIGNAL(projectionParamsChanged()), this, SLOT(slotLoadParams()));

  connect(ui->detSpotsEnabled, SIGNAL(toggled(bool)), projector, SLOT(enableSpots(bool)));
  connect(ui->renderAntialias, SIGNAL(toggled(bool)), this, SLOT(slotUpdateRenderHints()));
  connect(ui->renderAntialiasText, SIGNAL(toggled(bool)), this, SLOT(slotUpdateRenderHints()));

  QList<QPair<QDoubleSpinBox*, const char*> > l;
  QPair<QDoubleSpinBox*, const char*> pair;

  l << qMakePair(ui->detDist, SLOT(setDist(double)));
  l << qMakePair(ui->detWidth, SLOT(setWidth(double)));
  l << qMakePair(ui->detHeight, SLOT(setHeight(double)));
  l << qMakePair(ui->detOmega, SLOT(setOmega(double)));
  l << qMakePair(ui->detChi, SLOT(setChi(double)));
  l << qMakePair(ui->detPhi, SLOT(setPhi(double)));
  l << qMakePair(ui->detDx, SLOT(setXOffset(double)));
  l << qMakePair(ui->detDy, SLOT(setYOffset(double)));

  l << qMakePair(ui->detTextSize, SLOT(setTextSize(double)));
  l << qMakePair(ui->detSpotSize, SLOT(setSpotSize(double)));

  foreach (pair, l) {
    connect(pair.first, SIGNAL(valueChanged(double)), projector, pair.second);
  }

  slotLoadParams();
}

LauePlaneCfg::~LauePlaneCfg()
{
  delete ui;
}

void LauePlaneCfg::slotSetQRange() {
  projector->setWavevectors(ui->detMinQ->value(), ui->detMaxQ->value());
  ui->QDispA->setValue(2.0*M_PI/ui->detMaxQ->value());
  ui->QDispKEV->setValue(0.5*M_1_PI*12.398*ui->detMaxQ->value());
}

void LauePlaneCfg::slotLoadParams() {
  ui->detTextSize->setValue(projector->getTextSize());
  ui->detSpotSize->setValue(projector->getSpotSize());
  ui->maxRefLabel->setValue(projector->getMaxHklSqSum());
  ui->detMinQ->setValue(projector->Qmin());
  ui->detMaxQ->setValue(projector->Qmax());

  ui->detDist->setValue(projector->dist());
  ui->detWidth->setValue(projector->width());
  ui->detHeight->setValue(projector->height());
  ui->detOmega->setValue(projector->omega());
  ui->detChi->setValue(projector->chi());
  ui->detPhi->setValue(projector->phi());
  ui->detDx->setValue(projector->xOffset());
  ui->detDy->setValue(projector->yOffset());

  ui->detSpotsEnabled->setChecked(projector->spotsEnabled());

  QList<QGraphicsView*> l = projector->getScene()->views();
  if (l.size()) {
    ui->renderAntialias->setChecked(l.at(0)->renderHints()&QPainter::Antialiasing);
    ui->renderAntialiasText->setChecked(l.at(0)->renderHints()&QPainter::TextAntialiasing);
  }
}

void LauePlaneCfg::slotUpdateRenderHints() {
  foreach(QGraphicsView* view, projector->getScene()->views()) {
    view->setRenderHint(QPainter::Antialiasing, ui->renderAntialias->isChecked());
    view->setRenderHint(QPainter::TextAntialiasing, ui->renderAntialiasText->isChecked());
  }
}
