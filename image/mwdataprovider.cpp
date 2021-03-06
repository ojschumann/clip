/**********************************************************************
  Copyright (C) 2010 Gregory Tucker and Olaf J. Schumann

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

#include "mwdataprovider.h"
#include "image/dataproviderfactory.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QDateTime>
#include <QStringList>
 
#include <cmath>

#include "tools/xmltools.h"
#include "image/imagedatastore.h"

using namespace std;
/*
 Multiwire Image class for *.hs2 (and someday *.his) files
 written by Gregory Tucker supported by Ames Laboratory and Iowa State University
 based on basdataprovider written by Olaf Schumann

 version 1, March 21, 2011
*/

MWDataProvider::MWDataProvider(QObject* _parent) :
    DataProvider(_parent)
{
}

MWDataProvider::~MWDataProvider() {
}


QStringList MWDataProvider::Factory::fileFormatFilters() {
  //return QStringList() << "hs2" << "his"; //hs2 is 16-bit, his is 8-bit
  return QStringList() << "hs2";
}

DataProvider* MWDataProvider::Factory::getProvider(QString filename, ImageDataStore *store, QObject* _parent) {
  //load in information about the file
  QFileInfo info(filename);

  // Return if file does not exist
  if (!info.exists() || !info.isReadable()) return nullptr;

  // Check if suffix is .hs2 or .his
  QString suffix = info.suffix().toLower();
  //if (suffix!="hs2" || suffix!="his") return nullptr;
  if (suffix!="hs2") return nullptr;

  //load in the histogram image file
  QFile hs2File(info.filePath());

  if (!hs2File.open(QFile::ReadOnly)) return nullptr;

  QMap<QString, QVariant> headerData;

  int pixelCount = 256*256;

  QVector<float> pixelData(pixelCount);
  QList<int> overflowPixelPosition;
  quint16 maxValue=0;
  QDataStream in(&hs2File);
  in.setByteOrder(QDataStream::LittleEndian);
  quint16 pixel;
  for (int i=0; i<pixelCount; i++) {
    // Flip x coordinate
    int j = (i&0xFF00) | (0xFF-(i&0xFF));
    in >> pixel;
    if (pixel==0xFFFF) {
      overflowPixelPosition << j;
    } else if (pixel>maxValue) {
      maxValue = pixel;
    }
    pixelData[j]=log(1.0+pixel);
  }

  // set overflowed pixels to only slightly brighter than second highest value
  for (int i=0; i<overflowPixelPosition.size(); i++) {
    pixelData[overflowPixelPosition[i]] = log(2.0+maxValue);
  }


  /*
   after 256 * 256 * 2 bytes = 0x20000 Pixel data, there is more data. Up
   to now, the following has been decipered

   0x20000 -> 0x20033 zero terminated string with Sample Name
   0x20034 -> 0x20067 zero terminated string with Operator name
   0x20068 -> 0x2009c zero terminated string with Comment

   0x2019c -> 0x201cf zero terminated string with Original Filename

   0x202ac -> 0x202af detector-sample in cm distance as 4-byte float
   0x202b0 -> 0x202b1 exposure time in 1/10 s (uint16)

   0x202c0 -> 0x202c3 uint32 mA
   0x202c4 -> 0x202c7 uint32 kV

   0x20308 -> 0x2031f 6 4-byte floats with alpha, beta, gamma, a, b, c

  */


  headerData.insert("BitsPerPixel", 16);
  headerData.insert(Info_ImageSize, "256x256 pixels");  //pixels
  headerData.insert("Sample Description", QString(hs2File.read(52)));
  headerData.insert("Operator", QString(hs2File.read(52)));
  headerData.insert("Comment", QString(hs2File.read(52)));
  hs2File.seek(hs2File.pos()+256);
  headerData.insert("OriginalFilename", QString(hs2File.read(52)));

  float dist, alpha, beta, gamma, aLat, bLat, cLat;
  quint16 exposureTime;
  quint32 current, voltage;
  in.setFloatingPointPrecision(QDataStream::SinglePrecision);

  hs2File.seek(0x202ac);
  in >> dist >> exposureTime;
  hs2File.seek(0x202c0);
  in >> current >> voltage;

  hs2File.seek(0x20308);
  in >> alpha >> beta >> gamma >> aLat >> bLat >> cLat;

  store->setData(ImageDataStore::PixelSize, QSizeF(256.0, 256.0));
  store->setData(ImageDataStore::PhysicalSize, QSizeF(300.0, 300.0));
  store->setData(ImageDataStore::PlaneDetectorToSampleDistance, 10.0*dist);
  store->setData(ImageDataStore::CellA, aLat);
  store->setData(ImageDataStore::CellB, bLat);
  store->setData(ImageDataStore::CellC, cLat);
  store->setData(ImageDataStore::CellAlpha, alpha);
  store->setData(ImageDataStore::CellBeta, beta);
  store->setData(ImageDataStore::CellGamma, gamma);

  headerData.insert("Exposure", QString("%1 s at %2 kV, %3 mA").arg(0.1*exposureTime, 0, 'f', 1).arg(voltage).arg(current));
  headerData.insert("Sample-Detector distance", QString("%1 mm").arg(10.0*dist, 0, 'f', 1));
  headerData.insert("Cell", QString("%1 %2 %3 %4 %5 %6").arg(aLat).arg(bLat).arg(cLat).arg(alpha).arg(beta).arg(gamma));

  MWDataProvider* provider = new MWDataProvider(_parent);
  provider->insertFileInformation(filename);
  provider->providerInformation.unite(headerData);
  provider->pixelData = pixelData;
  return provider;
}

const void* MWDataProvider::getData() {
  return (void*)pixelData.data();
}

QSize MWDataProvider::size() {
  return QSize(256, 256);
}

int MWDataProvider::bytesCount() {
  return pixelData.size()*sizeof(float);
}

int MWDataProvider::pixelCount() {
  return pixelData.size();
}

DataProvider::Format MWDataProvider::format() {
  return Float32;
}


bool MWRegisterOK = DataProviderFactory::registerImageLoader(0, new MWDataProvider::Factory());
