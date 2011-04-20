#include "basdataprovider.h"
#include "image/dataproviderfactory.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QDateTime>
#include <QStringList>
#include <iostream>
#include <cmath>

#include "tools/xmltools.h"
#include "image/imagedatastore.h"

using namespace std;

BasDataProvider::BasDataProvider(QObject *parent) :
    DataProvider(parent)
{
}

BasDataProvider::~BasDataProvider() {
}


QStringList BasDataProvider::Factory::fileFormatFilters() {
  return QStringList() << "img" << "inf";
}

DataProvider* BasDataProvider::Factory::getProvider(QString filename, ImageDataStore* store, QObject *parent) {
  QFileInfo info(filename);

  // Return if file does not exist
  if (!info.exists() || !info.isReadable()) return NULL;

  // Check if suffix is .inf or .img
  QString suffix = info.suffix().toLower();
  if (suffix!="img" && suffix!="inf") return NULL;

  // search the second file (img or inf)
  QString secondSuffix = (suffix=="img")?"inf":"img";
  QDir dir(info.path());
  QStringList filter;
  filter << info.completeBaseName()+".*";
  QFileInfo info2;
  bool ok=false;
  foreach (QFileInfo i, dir.entryInfoList(filter, QDir::Files|QDir::Readable|QDir::CaseSensitive)) {
    if (i.suffix().toLower()==secondSuffix) {
      info2 = i;
      ok=true;
      break;
    }
  }
  if (!ok) return NULL;

  QFile infFile((suffix=="inf")?info.filePath():info2.filePath());
  QFile imgFile((suffix=="img")?info.filePath():info2.filePath());

  if (!infFile.open(QFile::ReadOnly) || !imgFile.open(QFile::ReadOnly)) return NULL;

  QTextStream inf(&infFile);

  if (inf.readLine()!="BAS_IMAGE_FILE") return NULL;

  QStringList keyValues;
  keyValues << "OriginalFilename" << "IP-Size" << "X-PixelSizeUM" << "Y-PixelSizeUM"
      << "BitsPerPixel" << "Width" << "Height" << "Sensitivity" << "Latitude" << "Exposure Date"
      << "UnixTime" << "OverflowPixels" << "" << "Comment";
  QStringList intKeyValues;
  intKeyValues << "X-PixelSizeUM" << "Y-PixelSizeUM"
      << "BitsPerPixel" << "Width" << "Height" << "Sensitivity" << "Latitude"
      << "UnixTime" << "OverflowPixels";


  QMap<QString, QVariant> headerData;
  foreach(QString key, keyValues) {
    if (inf.atEnd()) return NULL;
    QString s = inf.readLine();
    if (key!="") {
      if (intKeyValues.contains(key)) {
        bool ok;
        headerData.insert(key, QVariant(s.toInt(&ok)));
        if (!ok) return NULL;
      } else {
        headerData.insert(key, QVariant(s));
      }
    }
  }

  quint64 unixtime = headerData["UnixTime"].toULongLong();
  QDateTime date;
  date.setTime_t(unixtime);
  headerData["UnixTime"] = QVariant(QString("%1 (%2)").arg(unixtime).arg(date.toString(Qt::DefaultLocaleLongDate)));

  info.setFile(infFile.fileName());
  headerData.insert("InfFilename", QVariant(info.fileName()));
  headerData.insert("Complete Inf-Path", QVariant(info.canonicalFilePath()));


  int pixelCount = headerData["Width"].toInt()*headerData["Height"].toInt();
  int bytesPerPixel = (headerData["BitsPerPixel"].toInt()>8)?2:1;
  int dataSize = pixelCount * bytesPerPixel;
  if (dataSize != imgFile.size()) return NULL;


  QVector<float> pixelData(pixelCount);

  double linscale = 4000.0/headerData["Sensitivity"].toDouble();
  linscale *= headerData["X-PixelSizeUM"].toDouble()/100.0;
  linscale *= headerData["Y-PixelSizeUM"].toDouble()/100.0;

  double logscale = M_LN10*headerData["Latitude"].toDouble();
  linscale *= exp(-0.5*logscale);
  logscale /= (1<<headerData["BitsPerPixel"].toInt())-1;

  QDataStream in(&imgFile);
  in.setByteOrder(QDataStream::BigEndian);
  if (bytesPerPixel==1) {
    quint8 pixel;
    for (int i=0; i<pixelCount; i++) {
      in >> pixel;
      if (pixel!=0) {
        pixelData[i]=linscale * exp(logscale*pixel);
      }
    }
  } else if (bytesPerPixel==2) {
    quint16 pixel;
    for (int i=0; i<pixelCount; i++) {
      in >> pixel;
      if (pixel!=0) {
        //pixelData[i]=linscale * exp(logscale*pixel);
        pixelData[i]=1.0*pixel;
      }
    }
  }

  double w = headerData["Width"].toInt();
  double h = headerData["Height"].toInt();
  store->setData(ImageDataStore::PixelSize, QSizeF(w, h));
  store->setData(ImageDataStore::PhysicalSize, QSizeF(0.001*w*headerData["X-PixelSizeUM"].toInt(), 0.001*h*headerData["Y-PixelSizeUM"].toInt()));

  BasDataProvider* provider = new BasDataProvider(parent);
  provider->insertFileInformation(filename);
  provider->providerInformation.unite(headerData);
  provider->pixelData = pixelData;
  return provider;
}

const void* BasDataProvider::getData() {
  return (void*)pixelData.data();
}

QSize BasDataProvider::size() {
  return QSize(providerInformation["Width"].toInt(), providerInformation["Height"].toInt());
}

int BasDataProvider::bytesCount() {
  return pixelData.size()*sizeof(float);
}

int BasDataProvider::pixelCount() {
  return pixelData.size();
}

DataProvider::Format BasDataProvider::format() {
  return Float32;
}

bool BasRegisterOK = DataProviderFactory::registerImageLoader(0, new BasDataProvider::Factory());
