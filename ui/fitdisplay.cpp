#include "fitdisplay.h"
#include "ui_fitdisplay.h"

#include "core/crystal.h"
#include "core/projector.h"
#include "core/projectorfactory.h"

FitDisplay::FitDisplay(Crystal* c, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::FitDisplay),
    crystal(c)
{
  ui->setupUi(this);
}

FitDisplay::~FitDisplay()
{
  delete ui;
}

void FitDisplay::on_doFit_clicked()
{
  QList<FitObject*> fitObjects;

  Crystal* tmpCrystal = new Crystal(crystal);
  fitObjects << tmpCrystal;

  foreach (Projector* p, crystal->getConnectedProjectors()) {
    if ((p->zoneMarkers().size()>0) || (p->spotMarkers().size()>0)) {
      fitObjects << ProjectorFactory::getInstance().getProjector(p->projectorName());
    }
  }

}
