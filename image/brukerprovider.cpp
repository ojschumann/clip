#include "brukerprovider.h"
#include "image/dataproviderfactory.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QDateTime>
#include <QStringList>
#include <iostream>
#include <cmath>
#include <QtDebug>
#include "tools/xmltools.h"

using namespace std;

BrukerProvider::BrukerProvider(QObject *parent) :
    DataProvider(parent)
{
}

BrukerProvider::~BrukerProvider() {
}


QStringList BrukerProvider::Factory::fileFormatFilters() {
  return QStringList() << "sfrm";
}


template <typename T> QVector<unsigned int> readArrayFromSfrm(QFile& f, int len) {
  T val;
  QVector<unsigned int> data;
  data.reserve(len);

  QDataStream in(&f);
  in.setByteOrder(QDataStream::LittleEndian);

  for (int n=0; n<len; n++) {
    in >> val;
    data << val;
  }
  return data;
}

DataProvider* BrukerProvider::Factory::getProvider(QString filename, QObject *parent) {
  QFile imgFile(filename);

  if (!imgFile.open(QFile::ReadOnly)) return NULL;


  bool ok;
  int maxHeaderFields = 3;
  int headerSize = 0;
  QMap<QString, QVariant> headerData;
  for (int i=0; i<maxHeaderFields; i++) {
    QByteArray data = imgFile.read(80);
    QString headerfield(data);
    QString key = headerfield.left(7).simplified();
    QString value = headerfield.right(72).simplified();
    if (data[0]==char(0x1a) && data[1]==char(0x04)) {
      // padded -> end of header
      break;
    } else if (key=="HDRBLKS") {
      headerSize = 512*value.toInt();
      maxHeaderFields = headerSize/80;
    }

    if (key=="FORMAT") {
      headerData.insert(key, QVariant(value.toInt(&ok)));
      if (!ok) return NULL;
    } else {
      headerData.insert(key, QVariant(value));
    }
  }

  if ((headerSize==0) ||
      (!headerData.contains("FORMAT")) ||
      (!headerData.contains("NROWS")) ||
      (!headerData.contains("NCOLS")) ||
      (!headerData.contains("NOVERFL")) ||
      (!headerData.contains("NEXP")) ||
      (!headerData.contains("NPIXELB")))
    return NULL;

  int rows = headerData["NROWS"].toInt(&ok);
  if (!ok) return NULL;
  int cols = headerData["NCOLS"].toInt(&ok);
  if (!ok) return NULL;


  QStringList byteCounts = headerData["NPIXELB"].toString().split(' ');
  QStringList overflowNumbers = headerData["NOVERFL"].toString().split(' ');

  if (byteCounts.size()<1) return NULL;
  int bytesPerPixel = byteCounts.at(0).toInt(&ok);
  if (!ok) return NULL;

  int bytesPerUnderflow = 0;
  int numberUnderflow = 0;
  if (headerData["FORMAT"].toInt()>=100) {
    if (byteCounts.size()<2) return NULL;
    bytesPerUnderflow = byteCounts.at(1).toInt(&ok);
    if (!ok) return NULL;

    if (overflowNumbers.size()<1) return NULL;
    numberUnderflow = overflowNumbers.at(0).toInt(&ok);
    if (!ok) return NULL;
  }

  int bytesPerUnderflow = 0;
  int numberUnderflow = 0;
  if (headerData["FORMAT"].toInt()>=100) {
    if (byteCounts.size()<2) return NULL;
    bytesPerUnderflow = byteCounts.at(1).toInt(&ok);
    if (!ok) return NULL;

    if (overflowNumbers.size()<1) return NULL;
    // strangely sometimes zero...
    numberUnderflow = qMax(overflowNumbers.at(0).toInt(&ok), 0);
    if (!ok) return NULL;
  }

  int underflowTableSize = (bytesPerUnderflow * numberUnderflow);
  if (underflowTableSize%16!=0)
    underflowTableSize += 16 - underflowTableSize%16;


  int overflowSize = 0;
  if (headerData["FORMAT"].toInt()<100) {
    // TODO Parse this strange thing
    if (overflowNumbers.size()!=1) return NULL;
    overflowSize = 16*overflowNumbers.at(0).toInt(&ok);
    if (!ok) return NULL;
  } else {
    int twoByteOverflowCount = 0;
    int foutByteOverflowCount = 0;
    if (overflowNumbers.size()!=3) return NULL;
    if (bytesPerPixel==1) {
      twoByteOverflowCount = overflowNumbers.at(1).toInt(&ok);
      if (!ok) return NULL;
    }
    if (bytesPerPixel<=2) {
      fourByteOverflowCount = overflowNumbers.at(2).toInt(&ok);
      if (!ok) return NULL;
    }




  }





  QVector<unsigned int> pixelData;
  pixelData.reserve(rows*cols);
  imgFile.seek(512*headerData["HDRBLKS"].toInt());

  for (int n=0; n<cols*rows; n++) {
    if (bytesPerPixel==1) {
      quint8 val;
      in >> val;
      pixelData << val;
    } else if (bytesPerPixel==2) {
      quint16 val;
      in >> val;
      pixelData << val;
    } else if (bytesPerPixel==4) {
      quint32 val;
      in >> val;
      pixelData << val;
    }
  }

  for (int n=0; n<cols*rows; n++) {
  if ((bytesPerPixel==1 && pixelData.last()==0xFF) || (bytesPerPixel==2 && pixelData.last()==0xFFFF)) {
    qDebug() << "Overflow " << n;
  }


  BrukerProvider* provider = new BrukerProvider(parent);
  provider->insertFileInformation(filename);
  provider->providerInformation.unite(headerData);
  provider->pixelData = pixelData;

  return provider;
}

const void* BrukerProvider::getData() {
  return (void*)pixelData.data();
}

QSize BrukerProvider::size() {
  return QSize(providerInformation["NCOLS"].toInt(), providerInformation["NROWS"].toInt());
}

int BrukerProvider::bytesCount() {
  return pixelData.size()*sizeof(int);
}

int BrukerProvider::pixelCount() {
  return pixelData.size();
}

DataProvider::Format BrukerProvider::format() {
  return UInt32;
}

QSizeF BrukerProvider::absoluteSize() {
  return QSize();
  QSize s(size());
  return QSizeF(0.001*s.width()*providerInformation["X-PixelSizeUM"].toDouble(), 0.001*s.height()*providerInformation["Y-PixelSizeUM"].toDouble());
}


bool BrukerRegisterOK = DataProviderFactory::registerImageLoader(0, new BrukerProvider::Factory());
