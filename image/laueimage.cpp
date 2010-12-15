#include "laueimage.h"
#include <QImage>
#include <QPixmap>
#include <ui/imagetoolbox.h>
#include <ui/clip.h>
#include <QApplication>
#include <image/dataproviderfactory.h>
#include <image/datascalerfactory.h>
#include <iostream>

using namespace std;

LaueImage::LaueImage(QString s, QObject *parent) :
    QObject(parent), provider(0), scaler(0)
{
  provider = DataProviderFactory::getInstance().loadImage(s, this);
  if (provider) {
    scaler = DataScalerFactory::getInstance().getScaler(provider, this);
    if (scaler)
      connect(scaler, SIGNAL(imageContentsChanged()), this, SIGNAL(imageContentsChanged()));
  }
  valid = (provider!=0) && (scaler!=0);
  cout << "init LaueImage" << endl;
}

LaueImage::~LaueImage() {
  cout << "delete LaueImage" << endl;
}

QImage LaueImage::getScaledImage(const QSize& requestedSize, const QRectF& r) {
  return scaler->getImage(requestedSize, r);
}


QList<BezierCurve*> LaueImage::getTransferCurves() {
  return scaler->getTransferCurves();
}

void LaueImage::addTransform(const QTransform &t) {
  scaler->addTransform(t);
}

void LaueImage::resetAllTransforms() {
  scaler->resetAllTransforms();
}

bool LaueImage::hasAbsoluteSize() {
  return !provider->absoluteSize().isEmpty();
}

QSizeF LaueImage::absoluteSize() {
  return provider->absoluteSize();
}

QString LaueImage::name() {
  return provider->name();
}

QList<QWidget*> LaueImage::toolboxPages() {
  QList<QWidget*> pages;
  pages << provider->toolboxPages();
  return pages;
}
