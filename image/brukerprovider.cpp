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

template <typename DATA, typename TABLE, typename VALUE, typename F> bool replaceSpecialDataWithTableValue(DATA& data, const TABLE& table, VALUE specialValue, F f) {
  decltype(table.size()) tablePos=0;

  for (decltype(data.size()) n=0; n<data.size(); n++) {
    if (data.at(n)==specialValue) {
      if (tablePos>data.size()) return false;
      data[n]=table.at(tablePos++);
    } else {
      f(data[n]);
    }
  }
  return tablePos==table.size();
}

template <typename DATA, typename TABLE, typename VALUE> bool replaceSpecialDataWithTableValue(DATA& data, const TABLE& table, VALUE specialValue) {
  return replaceSpecialDataWithTableValue(data, table, specialValue, [](VALUE){});
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
    // Read one field, 80 bytes long, one colon, 72 bytes data
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
      headerSize = 512*value.toInt(); // header length is multiple of 512 bytes
      maxHeaderFields = headerSize/80;
    }

    // Store format as integer, reoccuring keys as one concatenated string and the rest as Variants
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

  // Number of rows and cols in the image
  int rows = headerData["NROWS"].toInt(&ok);
  if (!ok) return nullptr;
  int cols = headerData["NCOLS"].toInt(&ok);
  if (!ok) return nullptr;

  // Contains bytes per pixel for pixel data (and for unterflow table, if format>=100)
  QStringList byteCounts = headerData["NPIXELB"].toString().split(' ');
  if (byteCounts.size()<1) return nullptr;

  // Parse Bytes per Pixel
  int bytesPerPixel = byteCounts.at(0).toInt(&ok);
  if (!ok) return nullptr;

  // Calculate the size of data, overflow and underflow tables.
  int dataSize = cols*rows*bytesPerPixel;

  // if format>=100: Number of underflows, 2 bytes overflows and 4 bytes overflows
  // else just number of overflows
  QStringList overflowNumbers = headerData["NOVERFL"].toString().split(' ');
  if (overflowNumbers.size()<1) return nullptr;

  // Get number and size of entries in Underflowtable
  int bytesPerUnderflow = 0;
  int numberUnderflow = 0;
  if (headerData[Info_Format].toInt()>=100) {
    if (byteCounts.size()<2) return nullptr;
    bytesPerUnderflow = byteCounts.at(1).toInt(&ok);
    if (!ok) return nullptr;

    numberUnderflow = qMax(overflowNumbers.at(0).toInt(&ok), 0);
    if (!ok) return nullptr;
  }
  int underflowTableSize = padTo(bytesPerUnderflow * numberUnderflow, 16);

  // Get number and size of entries in Overflowtable
  int overflowTableSize = 0;
  if (headerData[Info_Format].toInt()<100) {
    if (overflowNumbers.size()!=1) return nullptr;
    overflowTableSize = padTo(16*overflowNumbers.at(0).toInt(&ok), 512);
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
  imgFile.seek(headerSize);
  QVector<unsigned int> pixelData = readArrayFromSfrm(imgFile, rows*cols, bytesPerPixel);

  int overflowSpecialAdd=0;
  // Handle Overflows
  if (headerData[Info_Format].toInt()<100) {
    // convert already checked...
    int overflowRecords = overflowNumbers.at(0).toInt();

    // Seek to overflow table
    imgFile.seek(headerSize+dataSize+underflowTableSize);

    // each record consists of 16 characters ascii numerical data,
    // first 9 characters for the true value and then 7 characters for the position to replace
    for (int n=0; n<overflowRecords; n++) {
      // read the two strings
      QByteArray valueStr = imgFile.read(9);
      QByteArray posStr = imgFile.read(7);

      //check, if read was successful
      if (valueStr.size()!=9 && posStr.size()!=7) return nullptr;

      // convert them to ints and check their range
      int value = valueStr.toInt(&ok);
      if (!ok or value<0) return nullptr;
      int pos = posStr.toInt(&ok);
      if (!ok or pos<0 or pos>pixelData.size()) return nullptr;

      // replace the overflown pixel
      pixelData[pos]=value;
    }

  } else {
    // 1 byte pixel data has 2 byte and 4 byte overflow tables
    // 2 byte pixel data has only 4 byte overflow table
    // 4 byte pixel data has no overflow table

    // Already checked, thus conversion is ok
    int twoByteOverflowCount = overflowNumbers.at(1).toInt();
    int fourByteOverflowCount = overflowNumbers.at(2).toInt();

    // build combined overflow table
    QVector<unsigned int> overflowData;
    unsigned int sigVal=0;
    if (bytesPerPixel==1) {
      // One byte per pixel value, thus two overflow tables. if pixel value is 0xFF, replace it with the next value
      // from the two byte Overflow table, if it is then 0xFFFF, replace it with the next value from
      // the four byte overflow table.

      // special value in pixelData is 0xFF
      sigVal = 0xFF;

      // Seek to two byte overflow table and read it
      imgFile.seek(headerSize+dataSize+underflowTableSize);
      overflowData = readArrayFromSfrm(imgFile, twoByteOverflowCount, 2);

      // seek to four byte overflow table and read it
      imgFile.seek(headerSize+dataSize+underflowTableSize+padTo(2*twoByteOverflowCount, 16));
      QVector<unsigned int> fourByteOverflowData = readArrayFromSfrm(imgFile, fourByteOverflowCount, 4);

      // Calculated precission images very large numbers in the 4 byte overflow table.
      // Presumabely a bug where the calculation module passes a signed array to the writer module expecting
      // an unsigned array, thus converting negative signed numbers to unsigned ones. these end up in very large numbers in the
      // overflow table e.g. 0xFFFFFFF8)
      // as Bugfix, find the lowest negative number here and add the absolute value later to the pixel data
      // (0xFFFFFFF8 == -8, 0xFFFFFFF8 + 8 == 0) thus shifting all numbers to the positive range.
      foreach (int v, fourByteOverflowData)
        if (v<0 && -v>overflowSpecialAdd) overflowSpecialAdd = -v;

      // replace the 0xFFFF in the two byte overflow table with the corresponding values in
      // four byte overflow table
      if (!replaceSpecialDataWithTableValue(overflowData, fourByteOverflowData, 0xFFFFu))
        return nullptr;
    } else if (bytesPerPixel==2) {
      // Two byte per pixel value, thus only one overflow table. Pixel values of 0xFFFF are replaced with the next value
      // from the four byte overflow table.

      // special value in pixelData is 0xFFFF
      sigVal = 0xFFFF;

      // seek to four byte overflow table and read it
      imgFile.seek(headerSize+dataSize+underflowTableSize);
      overflowData = readArrayFromSfrm(imgFile, fourByteOverflowCount, 4);
    }

    // replace special values in pixelData with items from the overflow table
    if (!replaceSpecialDataWithTableValue(pixelData, overflowData, sigVal))
      return nullptr;
  }

  // AFTER overflows handle underflows, the order is important
  // a pixel value of 0xFF has to be first replaced with the overflow value.
  // otherwise, the base line offset is added first, giving a different special value value
  if (numberUnderflow>0) { // Format is definitively >=100, as otherwise, underflow is not present
    // Read Baselineoffset as third entry in NEXP header
    QStringList exposureInfo = headerData["NEXP"].toString().split(' ');
    if (exposureInfo.size()<3) return nullptr;
    int exposureBaseline = exposureInfo.at(2).toInt(&ok);
    if (!ok) return nullptr;

    // Read Underflow table
    imgFile.seek(headerSize+dataSize);
    QVector<unsigned int> underflowData = readArrayFromSfrm(imgFile, numberUnderflow, bytesPerUnderflow);

    // Replace every occurence of 0 in pixeldata with an entry in the underflow table, increase every other pixel by exposureBaseline
    if (!replaceSpecialDataWithTableValue(pixelData, underflowData, 0u, [exposureBaseline](unsigned int& v){ v += exposureBaseline; } ))
        return nullptr;
  }

  for (int n=0; n<pixelData.size(); n++)
    pixelData[n]+=overflowSpecialAdd;


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
