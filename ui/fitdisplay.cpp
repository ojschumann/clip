#include "fitdisplay.h"
#include "ui_fitdisplay.h"

#include <iostream>

#include "refinement/fitparametermodel.h"
#include "refinement/neldermead.h"

using namespace std;

FitDisplay::FitDisplay(Crystal* c, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::FitDisplay),
    crystal(c)
{
  ui->setupUi(this);

  FitParameterModel* model = new FitParameterModel(crystal, this);
  ui->parameterView->setModel(model);
  fitter= new NelderMead(c, this);

  connect(ui->doFit, SIGNAL(clicked()), fitter, SLOT(start()));
  connect(ui->stopFit, SIGNAL(clicked()), fitter, SLOT(stop()));
}

FitDisplay::~FitDisplay()
{
  delete ui;
}


