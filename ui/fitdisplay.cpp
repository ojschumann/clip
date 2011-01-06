#include "fitdisplay.h"
#include "ui_fitdisplay.h"

#include <iostream>

#include "refinement/fitparametermodel.h"

using namespace std;

FitDisplay::FitDisplay(Crystal* c, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::FitDisplay),
    crystal(c)
{
  ui->setupUi(this);

  FitParameterModel* model = new FitParameterModel(crystal, this);
  ui->parameterView->setModel(model);

}

FitDisplay::~FitDisplay()
{
  delete ui;
}


