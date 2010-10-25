#include "stereocfg.h"
#include "ui_stereocfg.h"


StereoCfg::StereoCfg(StereoProjector* p, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StereoCfg)
{
  ui->setupUi(this);
  connect(p, SIGNAL(destroyed()), this, SLOT(slotProjectorClosed()));
  connect(ui->detTextSize, SIGNAL(valueChanged(double)), p, SLOT(setTextSize(double)));
  connect(ui->detSpotSize, SIGNAL(valueChanged(double)), p, SLOT(setSpotSize(double)));
  connect(ui->maxRefLabel, SIGNAL(valueChanged(int)), p, SLOT(setMaxHklSqSum(int)));
  connect(ui->detMinQ, SIGNAL(valueChanged(double)), this, SLOT(slotSetQRange()));
  connect(ui->detMaxQ, SIGNAL(valueChanged(double)), this, SLOT(slotSetQRange()));
}

StereoCfg::~StereoCfg()
{
  delete ui;
}

void StereoCfg::slotSetQRange() {
  projector->setWavevectors(0.5*M_1_PI*ui->detMinQ->value(), 0.5*M_1_PI*ui->detMaxQ->value());
  ui->QDispA->setValue(2*M_PI/ui->detMaxQ->value());
  ui->QDispKEV->setValue(0.5*M_1_PI*12.398*ui->detMaxQ->value());
}

void StereoCfg::slotProjectorClosed() {
  parent()->deleteLater();
}
