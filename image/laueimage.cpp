#include "laueimage.h"

#include <QImage>
#include <QPixmap>
#include <QApplication>
#include <iostream>
#include <QtConcurrentRun>

#include "ui/clip.h"
#include "image/dataproviderfactory.h"
#include "image/datascalerfactory.h"
#include "tools/xmltools.h"
#include "tools/zipiterator.h"
#include "image/BezierCurve.h"
#include "image/imagedatastore.h"

using namespace std;

LaueImage::LaueImage(QObject *parent) :
    QObject(parent), provider(0), scaler(0), dataStore()
{
  connect(&watcher, SIGNAL(finished()), this, SLOT(doneOpenFile()));
}


void LaueImage::startOpenFile(QString filename, QDomElement base) {
  watcher.setFuture(QtConcurrent::run(this, &LaueImage::doOpenFile, filename, base));
}

QPair<DataProvider*, DataScaler*> LaueImage::doOpenFile(QString filename, QDomElement base) {
  DataProvider* dp = DataProviderFactory::getInstance().loadImage(filename, &dataStore, this);
  DataScaler* ds = dp ? DataScalerFactory::getInstance().getScaler(dp, this) : NULL;
  if (ds && dp) {
    if (!base.isNull()) {
      dp->loadFromXML(base);
      ds->loadFromXML(base);
      loadCurvesFromXML(base, ds);
    }
    // dp and ds are created in this thread, but should live in the thread of LaueImage
    dp->moveToThread(thread());
    ds->moveToThread(thread());
  }
  return qMakePair(dp, ds);
}

void LaueImage::doneOpenFile() {
  DataProvider* dp = watcher.result().first;
  DataScaler* ds = watcher.result().second;
  if (dp && ds) {
    provider = dp;
    scaler = ds;
    //provider->setParent(this);
    //scaler->setParent(this);
    if (provider->parent()!=this) {
      cout << "boese..." << endl;
    }
    connect(scaler, SIGNAL(imageContentsChanged()), this, SIGNAL(imageContentsChanged()));
    connect(scaler, SIGNAL(histogramChanged(QVector<int>,QVector<int>,QVector<int>)), this, SIGNAL(histogramChanged(QVector<int>,QVector<int>,QVector<int>)));
  } else {
    if (dp) delete dp;
    if (ds) delete ds;
  }
  emit openFinished(this);
}

LaueImage::~LaueImage() {
  if (scaler) delete scaler;
  if (provider) delete provider;
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



