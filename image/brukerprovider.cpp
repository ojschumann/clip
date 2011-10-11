/**********************************************************************
  Copyright (C) 2008-2011 Olaf J. Schumann

  This file is part of the Cologne Laue Indexation Program.
  For more information, see <http://clip4.sf.net>

  Clip is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  Clip is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see http://www.gnu.org/licenses/
  or write to the Free Software Foundation, Inc., 51 Franklin Street,
  Fifth Floor, Boston, MA 02110-1301, USA.
 **********************************************************************/

#include "brukerprovider.h"
#include "image/dataproviderfactory.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QDateTime>
#include <QStringList>
 
#include <cmath>
#include <QtDebug>
#include "tools/xmltools.h"

#include "image/imagedatastore.h"

using namespace std;

QVector<unsigned int> readArrayFromSfrm(QFile& f, int len, int bytes);
int padTo(int value, int pad);

const char BrukerProvider::Info_Format[] = "FORMAT";

BrukerProvider::BrukerProvider(QObject* _parent) :
    DataProvider(_parent)
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

DataProvider* BrukerProvider::Factory::getProvider(QString filename, ImageDataStore *store, QObject* _parent) {
  QFile imgFile(filename);

  if (!imgFile.open(QFile::ReadOnly)) return nullptr;

  // Read header fields. Field #3 is HDRBLKS, read at least until this.

  bool ok;
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

    if (key==Info_Format) {
      headerData.insert(key, QVariant(value.toInt(&ok)));
      if (!ok) return nullptr;
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
      (!headerData.contains(Info_Format)) ||
      (!headerData.contains("NROWS")) ||
      (!headerData.contains("NCOLS")) ||
      (!headerData.contains("NOVERFL")) ||
      (!headerData.contains("NEXP")) ||
      (!headerData.contains("NPIXELB")))
    return nullptr;

  int rows = headerData["NROWS"].toInt(&ok);
  if (!ok) return nullptr;
  int cols = headerData["NCOLS"].toInt(&ok);
  if (!ok) return nullptr;


  QStringList byteCounts = headerData["NPIXELB"].toString().split(' ');
  QStringList overflowNumbers = headerData["NOVERFL"].toString().split(' ');
  if (byteCounts.size()<1) return nullptr;

  int bytesPerPixel = byteCounts.at(0).toInt(&ok);
  if (!ok) return nullptr;

  // Calculate the size of data, overflow and underflow tables.
  int dataSize = cols*rows*bytesPerPixel;

  int bytesPerUnderflow = 0;
  int numberUnderflow = 0;
  if (headerData[Info_Format].toInt()>=100) {
    if (byteCounts.size()<2) return nullptr;
    bytesPerUnderflow = byteCounts.at(1).toInt(&ok);
    if (!ok) return nullptr;

    if (overflowNumbers.size()<1) return nullptr;
    // strangely sometimes zero...
    numberUnderflow = qMax(overflowNumbers.at(0).toInt(&ok), 0);
    if (!ok) return nullptr;
  }

  int underflowTableSize = padTo(bytesPerUnderflow * numberUnderflow, 16);

  int overflowTableSize = 0;
  if (headerData[Info_Format].toInt()<100) {
    if (overflowNumbers.size()!=1) return nullptr;
    overflowTableSize = 16*overflowNumbers.at(0).toInt(&ok);
    if (!ok) return nullptr;
  } else {
    int twoByteOverflowCount = 0;
    int fourByteOverflowCount = 0;
    if (overflowNumbers.size()!=3) return nullptr;
    if (bytesPerPixel==1) {
      twoByteOverflowCount = overflowNumbers.at(1).toInt(&ok);
      if (!ok) return nullptr;
    }
    if (bytesPerPixel<=2) {
      fourByteOverflowCount = overflowNumbers.at(2).toInt(&ok);
      if (!ok) return nullptr;
    }
    overflowTableSize  = padTo(2*twoByteOverflowCount, 16);
    overflowTableSize += padTo(4*fourByteOverflowCount, 16);
  }

  // check is filesize matches calculated size
  if ((headerSize+dataSize+underflowTableSize+overflowTableSize)!=imgFile.size())
    return nullptr;

  // Read pixel data
  QVector<unsigned int> pixelData;
  imgFile.seek(headerSize);
  pixelData = readArrayFromSfrm(imgFile, rows*cols, bytesPerPixel);

  //TODO: check underflowdata
  if (headerData[Info_Format].toInt()<100) {
    // TODO implement old overflow format
  } else {
    int twoByteOverflowCount = overflowNumbers.at(1).toInt();
    int fourByteOverflowCount = overflowNumbers.at(2).toInt();
    QVector<unsigned int> overflowData;
    unsigned int sigVal=0;
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
  }

  store->setData(ImageDataStore::PixelSize, QSizeF(rows, cols));

  BrukerProvider* provider = new BrukerProvider(_parent);
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


bool BrukerRegisterOK = DataProviderFactory::registerImageLoader(0, new BrukerProvider::Factory());
