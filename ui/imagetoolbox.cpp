#include "imagetoolbox.h"
#include "ui_imagetoolbox.h"

#include "image/laueimage.h"
#include "tools/cropmarker.h"
#include "ui/resolutioncalculator.h"
#include "ui/contrastcurves.h"
#include "core/projector.h"

ImageToolbox::ImageToolbox(Projector* p, QWidget *parent):
    QMainWindow(parent),
    ui(new Ui::ImageToolbox),
    projector(p)
{
  ui->setupUi(this);

  if (projector && projector->getLaueImage()) {
    connect(projector->getLaueImage(), SIGNAL(destroyed()), this, SLOT(deleteLater()));

    ui->tabWidget->addTab(new ContrastCurves(projector->getLaueImage()), "Contrast");

    if (!projector->getLaueImage()->hasAbsoluteSize()) {
      ui->tabWidget->addTab(new ResolutionCalculator(projector->rulers(), projector->getLaueImage()), "Resolution");
    }

    foreach (QWidget* page, projector->getLaueImage()->toolboxPages()) {
      ui->tabWidget->addTab(page, page->objectName());
    }
  } else {
    deleteLater();
  }
  cout << "init ImageToolbox" << endl;
}

ImageToolbox::~ImageToolbox()
{
  cout << "delete imageToolbox" << endl;
  delete ui;
}


#include <tools/debug.h>
