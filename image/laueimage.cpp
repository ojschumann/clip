#include "laueimage.h"
#include <QImage>
#include <QPixmap>
#include <QApplication>
#include <iostream>

#include "ui/clip.h"
#include "image/dataproviderfactory.h"
#include "ui/imagetoolbox.h"
#include "image/datascalerfactory.h"
#include "tools/xmltools.h"

using namespace std;

LaueImage::LaueImage(QObject *parent) :
    QObject(parent), provider(0), scaler(0)
{
}

LaueImage::LaueImage(QString s, QObject *parent) :
    QObject(parent), provider(0), scaler(0)
{
  openFile(s);
}

void LaueImage::openFile(QString filename) {
  provider = DataProviderFactory::getInstance().loadImage(filename, this);
  if (provider) {
    scaler = DataScalerFactory::getInstance().getScaler(provider, this);
    if (scaler) {
      connect(scaler, SIGNAL(imageContentsChanged()), this, SIGNAL(imageContentsChanged()));
      connect(scaler, SIGNAL(histogramChanged(QVector<int>,QVector<int>,QVector<int>)), this, SIGNAL(histogramChanged(QVector<int>,QVector<int>,QVector<int>)));
    }
  }
  cout << "init LaueImage" << endl;
}

LaueImage::~LaueImage() {
  delete scaler;
  delete provider;
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
  pages << scaler->toolboxPages();
  return pages;
}

void LaueImage::saveToXML(QDomElement base) {
  QDomElement image = ensureElement(base, "Image");
  image.setAttribute("Filename", provider->getProviderInfo("Complete Path").toString());
  provider->saveToXML(image);
  scaler->saveToXML(image);
}

void LaueImage::loadFromXML(QDomElement base) {
  if (base.tagName()!="Image") return;
  QString filename = base.attribute("Filename");
  if (scaler) delete scaler;
  if (provider) delete provider;

  openFile(filename);
  if (isValid()) {
    provider->loadFromXML(base);
    scaler->loadFromXML(base);
  }
  cout << "loaded LaueImage" << endl;
}
