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


template <typename T> QVector<unsigned int> readArrayFromSfrmTmpl(QFile& f, int len) {
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

QVector<unsigned int> readArrayFromSfrm(QFile& f, int len, int bytes) {
  if (bytes==1) {
    return readArrayFromSfrmTmpl<quint8>(f, len);
  } else if (bytes==2) {
    return readArrayFromSfrmTmpl<quint16>(f, len);
  } else if (bytes==4) {
    return readArrayFromSfrmTmpl<quint32>(f, len);
  }
  return QVector<unsigned int>();
}

int padTo(int value, int pad) {
  int v = value%pad;
  if (v!=0)
    value += pad - v;
  return value;
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
      // padded -> indicates end of header
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

  int dataSize = cols*rows*bytesPerPixel;

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

  int underflowTableSize = padTo(bytesPerUnderflow * numberUnderflow, 16);

  int overflowTableSize = 0;
  if (headerData["FORMAT"].toInt()<100) {
    if (overflowNumbers.size()!=1) return NULL;
    overflowTableSize = 16*overflowNumbers.at(0).toInt(&ok);
    if (!ok) return NULL;
  } else {
    int twoByteOverflowCount = 0;
    int fourByteOverflowCount = 0;
    if (overflowNumbers.size()!=3) return NULL;
    if (bytesPerPixel==1) {
      twoByteOverflowCount = overflowNumbers.at(1).toInt(&ok);
      if (!ok) return NULL;
    }
    if (bytesPerPixel<=2) {
      fourByteOverflowCount = overflowNumbers.at(2).toInt(&ok);
      if (!ok) return NULL;
    }
    overflowTableSize  = padTo(2*twoByteOverflowCount, 16);
    overflowTableSize += padTo(4*fourByteOverflowCount, 16);
  }

  if ((headerSize+dataSize+underflowTableSize+overflowTableSize)!=imgFile.size())
    return NULL;


  QVector<unsigned int> pixelData;
  imgFile.seek(headerSize);
  pixelData = readArrayFromSfrm(imgFile, rows*cols, bytesPerPixel);

  //TODO: check underflowdata
  if (headerData["FORMAT"].toInt()<100) {
    // TODO implementold overflow format
  } else {
    int twoByteOverflowCount = overflowNumbers.at(1).toInt();
    int fourByteOverflowCount = overflowNumbers.at(2).toInt();
    QVector<unsigned int> overflowData;
    unsigned int sigVal;
    if (bytesPerPixel==1) {
      sigVal = 0xFF;
      imgFile.seek(headerSize+dataSize+underflowTableSize);
      overflowData = readArrayFromSfrm(imgFile, twoByteOverflowCount, 2);

      QVector<unsigned int> fourByteOverflowData;
      imgFile.seek(headerSize+dataSize+underflowTableSize+padTo(2*twoByteOverflowCount, 16));
      fourByteOverflowData = readArrayFromSfrm(imgFile, fourByteOverflowCount, 4);
      int i=0;
      for (int n=0; n<overflowData.size(); n++) {
        if (overflowData.at(n)==0xFFFF) {
          overflowData[n]=fourByteOverflowData.at(i++);
        }
      }
      qDebug() << i << " " << fourByteOverflowCount;
    } else if (bytesPerPixel==2) {
      sigVal = 0xFFFF;
      imgFile.seek(headerSize+dataSize+underflowTableSize);
      overflowData = readArrayFromSfrm(imgFile, fourByteOverflowCount, 4);
    }
    int i=0;
    for (int n=0; n<pixelData.size(); n++) {
      if (pixelData.at(n)==sigVal) {
        pixelData[n]=overflowData.at(i++);
      }
    }
    qDebug() << i << " " << twoByteOverflowCount;
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
