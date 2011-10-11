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

#include "basdataprovider.h"
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

const char BasDataProvider::Info_OriginalFilename[] = "OriginalFilename";
const char BasDataProvider::Info_IPSize[] = "IP-Size";
const char BasDataProvider::Info_XPixelSize[] = "X-PixelSizeUM";
const char BasDataProvider::Info_YPixelSize[] = "Y-PixelSizeUM";
const char BasDataProvider::Info_BitsPerPixel[] = "BitsPerPixel";
const char BasDataProvider::Info_Width[] = "Width";
const char BasDataProvider::Info_Height[] = "Height";
const char BasDataProvider::Info_Sensitivity[] = "Sensitivity";
const char BasDataProvider::Info_Latitude[] = "Latitude";
const char BasDataProvider::Info_ExposureDate[] = "Exposure Date";
const char BasDataProvider::Info_UnixTime[] = "UnixTime";
const char BasDataProvider::Info_OverflowPixels[] = "OverflowPixels";
const char BasDataProvider::Info_Comment[] = "Comment";
const char BasDataProvider::Info_InfFilename[] = "InfFilename";
const char BasDataProvider::Info_InfPath[] = "Complete Inf-Path";
const char BasDataProvider::Info_PixelSize[] = "PixelSize";
const char BasDataProvider::INF_Suffix[] = "inf";
const char BasDataProvider::IMG_Suffix[] = "img";


BasDataProvider::BasDataProvider(QObject* _parent) :
    DataProvider(_parent)
{
}

BasDataProvider::~BasDataProvider() {
}


QStringList BasDataProvider::Factory::fileFormatFilters() {
  return QStringList() << IMG_Suffix << INF_Suffix;
}

DataProvider* BasDataProvider::Factory::getProvider(QString filename, ImageDataStore* store, QObject* _parent) {
  QFileInfo info(filename);

  // Return if file does not exist
  if (!info.exists() || !info.isReadable()) return nullptr;

  // Check if suffix is .inf or .img
  QString suffix = info.suffix().toLower();
  if (suffix!=IMG_Suffix && suffix!=INF_Suffix) return nullptr;

  // search the second file (img or inf)
  QString secondSuffix = (suffix==IMG_Suffix)?INF_Suffix:IMG_Suffix;
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
  if (!ok) return nullptr;

  QFile infFile((suffix==INF_Suffix)?info.filePath():info2.filePath());
  QFile imgFile((suffix==IMG_Suffix)?info.filePath():info2.filePath());

  if (!infFile.open(QFile::ReadOnly) || !imgFile.open(QFile::ReadOnly)) return nullptr;

  QTextStream inf(&infFile);

  if (inf.readLine()!="BAS_IMAGE_FILE") return nullptr;

  QStringList keyValues;
  keyValues << Info_OriginalFilename << Info_IPSize << Info_XPixelSize << Info_YPixelSize
      << Info_BitsPerPixel << Info_Width << Info_Height << Info_Sensitivity << Info_Latitude << Info_ExposureDate
      << Info_UnixTime << Info_OverflowPixels << "" << Info_Comment;
  QStringList intKeyValues;
  intKeyValues << Info_XPixelSize << Info_YPixelSize
      << Info_BitsPerPixel << Info_Width << Info_Height << Info_Sensitivity << Info_Latitude
      << Info_UnixTime << Info_OverflowPixels;


  QMap<QString, QVariant> headerData;
  foreach(QString key, keyValues) {
    if (inf.atEnd()) return nullptr;
    QString s = inf.readLine();
    if (key!="") {
      if (intKeyValues.contains(key)) {
        bool ok;
        headerData.insert(key, QVariant(s.toInt(&ok)));
        if (!ok) return nullptr;
      } else {
        headerData.insert(key, QVariant(s));
      }
    }
  }

  quint64 unixtime = headerData[Info_UnixTime].toULongLong();
  QDateTime date;
  date.setTime_t(unixtime);
  headerData[Info_UnixTime] = QVariant(QString("%1 (%2)").arg(unixtime).arg(date.toString(Qt::DefaultLocaleLongDate)));

  info.setFile(infFile.fileName());
  headerData.insert(Info_InfFilename, QVariant(info.fileName()));
  headerData.insert(Info_InfPath, QVariant(info.canonicalFilePath()));


  int pixelCount = headerData[Info_Width].toInt()*headerData[Info_Height].toInt();
  int bytesPerPixel = (headerData[Info_BitsPerPixel].toInt()>8)?2:1;
  int dataSize = pixelCount * bytesPerPixel;
  if (dataSize != imgFile.size()) return nullptr;


  QVector<float> pixelData(pixelCount);

  double linscale = 4000.0/headerData[Info_Sensitivity].toDouble();
  linscale *= headerData[Info_XPixelSize].toDouble()/100.0;
  linscale *= headerData[Info_YPixelSize].toDouble()/100.0;

  double logscale = M_LN10*headerData[Info_Latitude].toDouble();
  linscale *= exp(-0.5*logscale);
  logscale /= (1<<headerData[Info_BitsPerPixel].toInt())-1;

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

  int w = headerData[Info_Width].toInt();
  int h = headerData[Info_Height].toInt();
  int pixX = headerData[Info_XPixelSize].toInt();
  int pixY = headerData[Info_YPixelSize].toInt();
  store->setData(ImageDataStore::PixelSize, QSizeF(w, h));
  store->setData(ImageDataStore::PhysicalSize, QSizeF(0.001*w*pixX, 0.001*h*pixY));

  headerData.insert(Info_ImageSize, QString("%1x%2 pixels").arg(w).arg(h));
  headerData.insert(Info_PixelSize, QString("%1x%2 µm").arg(pixX).arg(pixY));

  headerData.remove(Info_Width);
  headerData.remove(Info_Height);
  headerData.remove(Info_XPixelSize);
  headerData.remove(Info_YPixelSize);

  BasDataProvider* provider = new BasDataProvider(_parent);
  provider->insertFileInformation(filename);
  provider->providerInformation.unite(headerData);
  provider->pixelData = pixelData;
  provider->dataSize = QSize(w,h);
  return provider;
}

const void* BasDataProvider::getData() {
  return (void*)pixelData.data();
}

QSize BasDataProvider::size() {
  return dataSize;
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
