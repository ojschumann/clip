/**********************************************************************
  Copyright (C) 2010 Gregory Tucker and Olaf J. Schumann

  This file is part of the Cologne Laue Indexation Program.
  For more information, see <http://clip.berlios.de>

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
#include <iostream>
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

MWDataProvider::MWDataProvider(QObject *parent) :
    DataProvider(parent)
{
}

MWDataProvider::~MWDataProvider() {
}


QStringList MWDataProvider::Factory::fileFormatFilters() {
  //return QStringList() << "hs2" << "his"; //hs2 is 16-bit, his is 8-bit
  return QStringList() << "hs2";
}

DataProvider* MWDataProvider::Factory::getProvider(QString filename, ImageDataStore *store, QObject *parent) {
  //load in information about the file
  QFileInfo info(filename);

  // Return if file does not exist
  if (!info.exists() || !info.isReadable()) return NULL;

  // Check if suffix is .hs2 or .his
  QString suffix = info.suffix().toLower();
  //if (suffix!="hs2" || suffix!="his") return NULL;
  if (suffix!="hs2") return NULL;

  //load in the histogram image file
  QFile hs2File(info.filePath());

  if (!hs2File.open(QFile::ReadOnly)) return NULL;

  QMap<QString, QVariant> headerData;
  double xpxsize = 300000./256; // x-pixel size in microns
  headerData.insert("X-PixelSizeUM",xpxsize);
  double ypxsize = 300000./256; // y-pixel size in microns
  headerData.insert("Y-PixelSizeUM",ypxsize);

  headerData.insert("BitsPerPixel", 16);
  headerData.insert("Width",256);  //pixels
  headerData.insert("Height",256); //pixels

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
  There is more useful data stored after the image, however I'm not sure of it's exact format 
  so I've left it out at the moment. Among other things, the user-defined sample-to-detector
  distance is stored in every hs2 file; if this was read automatically it could save on one
  small annoyance.
  */


  headerData.insert("Sample Description", QString(hs2File.read(52)));
  headerData.insert("Experimenter", QString(hs2File.read(52)));
  headerData.insert("Comment", QString(hs2File.read(52)));
  hs2File.seek(hs2File.pos()+256);
  headerData.insert("OriginalFilename", QString(hs2File.read(52)));

  // Colimator diameter in mm at 775 as float?
  // Detector width and height in cm possible at 779 and 783 as float
  // Sample-detector distance in mm possibly at 787 as float
  hs2File.seek(hs2File.pos()+775-256-4*52);
  float colDia, dist, width, height;
  in.setFloatingPointPrecision(QDataStream::SinglePrecision);
  in.setByteOrder(QDataStream::BigEndian);
  in >> colDia >> width >> height >> dist;

  store->setData(ImageDataStore::PixelSize, QSizeF(256.0, 256.0));
  store->setData(ImageDataStore::PhysicalSize, QSizeF(10.0*width, 10.0*height));
  store->setData(ImageDataStore::PlaneDetectorToSampleDistance, dist);

  MWDataProvider* provider = new MWDataProvider(parent);
  provider->insertFileInformation(filename);
  provider->providerInformation.unite(headerData);
  provider->pixelData = pixelData;
  return provider;
}

const void* MWDataProvider::getData() {
  return (void*)pixelData.data();
}

QSize MWDataProvider::size() {
  return QSize(providerInformation["Width"].toInt(), providerInformation["Height"].toInt());
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
