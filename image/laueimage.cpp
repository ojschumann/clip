#include "laueimage.h"
#include <QImage>
#include <QPixmap>
#include <QApplication>
#include <iostream>
#include <QtConcurrentRun>

#include "ui/clip.h"
#include "image/dataproviderfactory.h"
#include "ui/imagetoolbox.h"
#include "image/datascalerfactory.h"
#include "tools/xmltools.h"
#include "tools/zipiterator.h"
#include "image/BezierCurve.h"

using namespace std;

LaueImage::LaueImage(QObject *parent) :
    QObject(parent), provider(0), scaler(0)
{
  cout << "init LaueImage" << endl;
  connect(&watcher, SIGNAL(finished()), this, SLOT(doneOpenFile()));

}


void LaueImage::startOpenFile(QString filename, QDomElement base) {
  watcher.setFuture(QtConcurrent::run(this, &LaueImage::doOpenFile, filename, base));
}

QPair<DataProvider*, DataScaler*> LaueImage::doOpenFile(QString filename, QDomElement base) {
  DataProvider* dp = DataProviderFactory::getInstance().loadImage(filename);
  DataScaler* ds = NULL;
  if (dp) {
    ds  = DataScalerFactory::getInstance().getScaler(dp);
  }
  if (ds && dp && !base.isNull()) {
    dp->loadFromXML(base);
    ds->loadFromXML(base);
    loadCurvesFromXML(base, ds);
  }
  // dp and ds are created in this thread, but should live in the thread of LaueImage
  dp->moveToThread(thread());
  ds->moveToThread(thread());
  return qMakePair(dp, ds);
}

void LaueImage::doneOpenFile() {
  cout << "Done with open" << endl;
  DataProvider* dp = watcher.result().first;
  DataScaler* ds = watcher.result().second;
  if (dp && ds) {
    provider = dp;
    scaler = ds;
    provider->setParent(this);
    scaler->setParent(this);
    connect(scaler, SIGNAL(imageContentsChanged()), this, SIGNAL(imageContentsChanged()));
    connect(scaler, SIGNAL(histogramChanged(QVector<int>,QVector<int>,QVector<int>)), this, SIGNAL(histogramChanged(QVector<int>,QVector<int>,QVector<int>)));
    emit openFinished(this);
  } else {
    if (dp) delete dp;
    if (ds) delete ds;
  }
}

LaueImage::~LaueImage() {
  delete scaler;
  delete provider;
  cout << "delete LaueImage" << endl;
}

QImage LaueImage::getScaledImage(const QSize& requestedSize, const QPolygonF& r) {
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

QSize LaueImage::originalSize() {
  return provider->size();
}

QSizeF LaueImage::originalAbsoluteSize() {
  return provider->absoluteSize();
}

QSizeF LaueImage::transformedSize() {
  return scaler->transformedSize();
}

QSizeF LaueImage::transformedAbsoluteSize() {
  return scaler->transformedAbsoluteSize();
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

const char XML_LaueImage_element[] = "Image";
const char XML_LaueImage_element_fn[] = "Filename";

void LaueImage::saveToXML(QDomElement base) {
  QDomElement image = ensureElement(base, XML_LaueImage_element);
  image.setAttribute(XML_LaueImage_element_fn, provider->getProviderInfo("Complete Path").toString());
  provider->saveToXML(image);
  scaler->saveToXML(image);
  saveCurvesToXML(image);
}

void LaueImage::loadFromXML(QDomElement base) {
  QDomElement element = base.elementsByTagName(XML_LaueImage_element).at(0).toElement();
  if (element.isNull()) return;
  QString filename = element.attribute(XML_LaueImage_element_fn);
  if (scaler) delete scaler;
  if (provider) delete provider;

  startOpenFile(filename, element);
}


const char XML_laueImage_Transfercurves[] = "Transfercurves";
QStringList TransfercurveNames = QStringList() << "Value" << "Red" << "Green" << "Blue";

void LaueImage::saveCurvesToXML(QDomElement base) {
  QDomElement curves = ensureElement(base, XML_laueImage_Transfercurves);
  QPair<BezierCurve*, QString> p;
  foreach (p, Zip(getTransferCurves(), TransfercurveNames)) {
    p.first->saveToXML(curves, p.second);
  }
}

void LaueImage::loadCurvesFromXML(QDomElement base, DataScaler* ds) {
  if (!ds) ds=scaler;
  QDomElement curves = base.elementsByTagName(XML_laueImage_Transfercurves).at(0).toElement();
  if (curves.isNull()) return;
  QPair<BezierCurve*, QString> p;
  foreach (p, Zip(ds->getTransferCurves(), TransfercurveNames)) {
    p.first->loadFromXML(curves, p.second);
  }
}
