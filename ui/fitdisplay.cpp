#include "fitdisplay.h"
#include "ui_fitdisplay.h"

#include <iostream>

using namespace std;

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


