/**********************************************************************
  Copyright (C) 2010 Olaf J. Schumann

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

#include "xyzdataprovider.h"

#include <QFile>
#include <QStringList>
#include <QDataStream>
 

#include "image/dataproviderfactory.h"
#include "image/imagedatastore.h"

using namespace std;

XYZDataProvider::XYZDataProvider(QObject *parent) :
    DataProvider(parent)
{
}

XYZDataProvider::~XYZDataProvider() {
}


QStringList XYZDataProvider::Factory::fileFormatFilters() {
  return QStringList() << "raw" << "xyz";
}

DataProvider* XYZDataProvider::Factory::getProvider(QString filename, ImageDataStore *store, QObject *parent) {
  QFile imgFile(filename);

  if (!imgFile.open(QFile::ReadOnly)) return NULL;

  QDataStream in(&imgFile);
  in.setByteOrder(QDataStream::LittleEndian);

  short unsigned int tmp;

  in >> tmp;
  int width = tmp;
  in >> tmp;
  int height = tmp;

  if ((2*width*height+4)!=imgFile.size()) return NULL;
  QVector<float> pixelData;
  pixelData.reserve(width*height);


  for (int i=0; i<width*height; i++) {
    in >> tmp;
    pixelData << static_cast<float>(tmp);
  }

  store->setData(ImageDataStore::PixelSize, QSizeF(width, height));

  XYZDataProvider* provider = new XYZDataProvider(parent);
  provider->pixelData = pixelData;
  provider->imgWidth = width;
  provider->imgHeight = height;
  provider->insertFileInformation(filename);
  provider->providerInformation.insert(Info_ImageSize, QString("%1x%2 pixels").arg(width).arg(height));

  return provider;
}


const void* XYZDataProvider::getData() {
  return (void*)pixelData.data();
}

QSize XYZDataProvider::size() {
  return QSize(imgWidth, imgHeight);
}

int XYZDataProvider::bytesCount() {
  return pixelData.size()*sizeof(float);
}

int XYZDataProvider::pixelCount() {
  return pixelData.size();
}

DataProvider::Format XYZDataProvider::format() {
  return Float32;
}

void XYZDataProvider::saveToXML(QDomElement) {

}

void XYZDataProvider::loadFromXML(QDomElement) {

}

bool XYZRegisterOK = DataProviderFactory::registerImageLoader(192, new XYZDataProvider::Factory());
