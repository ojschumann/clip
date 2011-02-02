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
  bool ok;
  QFile imgFile(filename);

  if (!imgFile.open(QFile::ReadOnly)) return NULL;

  // Read header fields. Field #3 is HDRBLKS, read at least until this.
  int maxHeaderFields = 3;
  int headerSize = 0;
  QMap<QString, QVariant> headerData;
  for (int i=0; i<maxHeaderFields; i++) {
    // Read one field
    QByteArray data = imgFile.read(80);
    QString headerfield(data);
    // Split to 7 character key and 72 character value (omiting one colon)
    QString key = headerfield.left(7).simplified();
    QString value = headerfield.right(72).simplified();
    if (data[0]==char(0x1a) && data[1]==char(0x04)) {
      // padded -> indicates end of header
      break;
    } else if (key=="HDRBLKS") {
      // If HDRBLKS is found, set headersize and maximal headerfields nr acordingly
      headerSize = 512*value.toInt();
      maxHeaderFields = headerSize/80;
    }

    if (key=="FORMAT") {
      headerData.insert(key, QVariant(value.toInt(&ok)));
      if (!ok) return NULL;
    } else {
      if (headerData.contains(key)) {
        headerData[key] = QVariant(headerData[key].toString() + " " + value);
      } else {
        headerData.insert(key, QVariant(value));
      }
    }
  }

  // Check, if all nesessary fields are present
  if ((headerSize==0) ||
      (!headerData.contains("FORMAT")) ||
      (!headerData.contains("NROWS")) ||
      (!headerData.contains("NCOLS")) ||
      (!headerData.contains("NOVERFL")) ||
      (!headerData.contains("NEXP")) ||
      (!headerData.contains("NPIXELB")))
    return NULL;


  QStringList byteCounts = headerData["NPIXELB"].toString().split(' ');
  QStringList overflowNumbers = headerData["NOVERFL"].toString().split(' ');
  if (byteCounts.size()<1) return NULL;

  int bytesPerPixel = byteCounts.at(0).toInt(&ok);
  if (!ok) return NULL;
  int rows = headerData["NROWS"].toInt(&ok);
  if (!ok) return NULL;
  int cols = headerData["NCOLS"].toInt(&ok);
  if (!ok) return NULL;

  // Calculate the size of data, overflow and underflow tables.
  int dataSize = cols*rows*bytesPerPixel;

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

  int underflowTableSize = padTo(bytesPerUnderflow * qMax(0, numberUnderflow), 16);

  int overflowTableSize = 0;
  if (headerData["FORMAT"].toInt()<100) {
    if (overflowNumbers.size()!=1) return NULL;
    overflowTableSize = 16*overflowNumbers.at(0).toInt(&ok);
    if (!ok) return NULL;
  } else {
    if (overflowNumbers.size()!=3) return NULL;
    if (bytesPerPixel==1) {
      overflowTableSize += padTo(2*overflowNumbers.at(1).toInt(&ok), 16);
      if (!ok) return NULL;
    }
    if (bytesPerPixel<=2) {
      overflowTableSize += padTo(4*overflowNumbers.at(2).toInt(&ok), 16);
      if (!ok) return NULL;
    }
  }

  // check is filesize matches calculated size
  if ((headerSize+dataSize+underflowTableSize+overflowTableSize)!=imgFile.size())
    return NULL;

  // Read pixel data
  QVector<unsigned int> pixelData;
  imgFile.seek(headerSize);
  pixelData = readArrayFromSfrm(imgFile, rows*cols, bytesPerPixel);

  // TODO: Remove debug stuff
  int zeroCount = 0;
  int maxCount = 0;
  foreach(unsigned int n, pixelData) {
    if (n==0) {
      zeroCount++;
    } else if (n==0xFF) {
      maxCount++;
    }
  }
  qDebug() << "zeroMax: " << zeroCount << " " << maxCount;

  //TODO: check underflowdata
  if (headerData["FORMAT"].toInt()<100) {
    // TODO implement old overflow format
  } else {
    // Read underflow table
    imgFile.seek(headerSize+dataSize);
    QList<unsigned int> underflowData = readArrayFromSfrm(imgFile, qMax(0, numberUnderflow), bytesPerUnderflow).toList();
    int baselineOffset = numberUnderflow>=0 ? headerData["NEXP"].toString().split(' ').at(2).toInt(&ok) : 0;
    if (!ok) return NULL;

    // Read overflow tables (up to two)
    int twoByteOverflowCount = overflowNumbers.at(1).toInt();
    imgFile.seek(headerSize+dataSize+underflowTableSize);
    QList<unsigned int> twoByteoverflowData = readArrayFromSfrm(imgFile, twoByteOverflowCount, 2).toList();

    int fourByteOverflowCount = overflowNumbers.at(2).toInt();
    imgFile.seek(headerSize+dataSize+underflowTableSize+padTo(2*twoByteOverflowCount, 16));
    QList<unsigned int> fourByteOverflowData = readArrayFromSfrm(imgFile, fourByteOverflowCount, 4).toList();

    // do set Under- and Overflow
    for (int n=0; n<pixelData.size(); n++) {
      int val = pixelData.at(n);
      if (val==0) {
        if (numberUnderflow>=0) {
          if (underflowData.isEmpty()) return NULL;
          val=underflowData.takeFirst();
        }
      } else {
        if ((bytesPerPixel==1) && (val==0xFF)) {
          if (twoByteoverflowData.isEmpty()) return NULL;
          val=twoByteoverflowData.takeFirst();
        }
        if ((bytesPerPixel<=2) && (val==0xFFFF)) {
          if (fourByteOverflowData.isEmpty()) return NULL;
          val=fourByteOverflowData.takeFirst();
        }
      }
      pixelData[n] = val + baselineOffset;
    }
    qDebug() << "Size of tables" << underflowData.size() << twoByteoverflowData.size() << fourByteOverflowData.size();
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
