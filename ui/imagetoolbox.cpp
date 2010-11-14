#include "imagetoolbox.h"
#include "ui_imagetoolbox.h"

#include "image/laueimage.h"
#include "tools/cropmarker.h"
#include "ui/resolutioncalculator.h"
#include "ui/contrastcurves.h"

ImageToolbox::ImageToolbox(Projector* p, QWidget *parent):
    QMainWindow(parent),
    ui(new Ui::ImageToolbox),
    projector(p)
{
  ui->setupUi(this);
  connect(projector->getLaueImage(), SIGNAL(destroyed()), this, SLOT(deleteLater()));

  ui->tabWidget->addTab(new ResolutionCalculator(projector), "Resolution");
  ui->tabWidget->addTab(new ContrastCurves(projector), "Contrast");

  cout << "init ImageToolbox" << endl;
}

ImageToolbox::~ImageToolbox()
{
  cout << "delete imageToolbox" << endl;
  delete ui;
}


#include <tools/debug.h>

void ImageToolbox::on_doCrop_clicked()
{
  QPolygonF poly = projector->getCropMarker()->getRect();
  foreach (QPointF p, poly)
    printPoint("lala", p);
  QTransform t;
  if (QTransform::quadToSquare(poly, t)) {
    projector->doImgRotation(t);
  }
}

void ImageToolbox::on_pushButton_clicked()
{
  if (projector && projector->getLaueImage())
    projector->getLaueImage()->resetAllTransforms();
}

void ImageToolbox::on_pushButton_2_clicked()
{
  if (projector )
    projector->delCropMarker();

}

void ImageToolbox::on_actionCrop_triggered() {
  projector->showCropMarker();
}
