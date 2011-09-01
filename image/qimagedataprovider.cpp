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

#include "qimagedataprovider.h"

#include <QStringList>
#include <QMap>
 
#include <QImageReader>

#include "image/dataproviderfactory.h"
#include "image/imagedatastore.h"

using namespace std;


QImageDataProvider::QImageDataProvider(const QImage& img, QObject *parent) :
    DataProvider(parent),
    data(img)
{
}

QImageDataProvider::~QImageDataProvider() {}

QStringList QImageDataProvider::Factory::fileFormatFilters() {
  QStringList formats;
  foreach (QByteArray format, QImageReader::supportedImageFormats()) {
    formats += QString(format);
  }
  return formats;
}

DataProvider* QImageDataProvider::Factory::getProvider(QString filename, ImageDataStore *store, QObject *parent) {
  QImage img(filename);
  if (!img.isNull()) {
    QMap<QString, QVariant> headerData;
    foreach (QString key, img.textKeys()) {
      if (key!="")
        headerData.insert(key, QVariant(img.text(key)));
    }

    store->setData(ImageDataStore::PixelSize, img.size());

    headerData.insert(Info_ImageSize, QString("%1x%2 pixels").arg(img.width()).arg(img.height()));
    QImageDataProvider* provider = new QImageDataProvider(img.convertToFormat(QImage::Format_ARGB32_Premultiplied), parent);
    provider->insertFileInformation(filename);
    provider->providerInformation.unite(headerData);
    return provider;
  }
  return NULL;
}

const void* QImageDataProvider::getData() {
  return data.bits();
}

QSize QImageDataProvider::size() {
  return data.size();
}

int QImageDataProvider::bytesCount() {
  return data.byteCount();
}

int QImageDataProvider::pixelCount() {
  return data.width()*data.height();
}

DataProvider::Format QImageDataProvider::format() {
  return RGB8Bit;
}

void QImageDataProvider::saveToXML(QDomElement) {

}

void QImageDataProvider::loadFromXML(QDomElement) {

}

bool registerOK = DataProviderFactory::registerImageLoader(128, new QImageDataProvider::Factory());
