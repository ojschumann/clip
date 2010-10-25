#include "stereocfg.h"
#include "ui_stereocfg.h"


StereoCfg::StereoCfg(StereoProjector* p, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StereoCfg)
{
  ui->setupUi(this);
  connect(p, SIGNAL(destroyed()), this, SLOT(slotProjectorClosed()));
  connect(ui->detTextSize, SIGNAL(valueChanged(double)), p, SLOT(setTextSize(double)));
  connect(ui->maxRefLabel, SIGNAL(valueChanged(int)), p, SLOT(setMaxHklSqSum(int)));
}

StereoCfg::~StereoCfg()
{
  delete ui;
}

void StereoCfg::slotProjectorClosed() {
  parent()->deleteLater();
}
