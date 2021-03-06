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

#include "dataproviderfactory.h"

 
#include <QStringList>

using namespace std;


DataProviderFactory::DataProviderFactory()
{
}

DataProviderFactory::DataProviderFactory(const DataProviderFactory &) {};

DataProviderFactory::~DataProviderFactory() {
  foreach (DataProvider::ImageFactoryClass* item, imageLoaders.values()) {
    delete item;
  }
}

DataProviderFactory& DataProviderFactory::getInstance() {
  static DataProviderFactory instance;
  return instance;
}

DataProvider* DataProviderFactory::loadImage(const QString &filename, ImageDataStore* store, QObject* _parent) {
  foreach (int key, imageLoaders.uniqueKeys()) {
    foreach (auto loader, imageLoaders.values(key)) {
      DataProvider* dp = loader->getProvider(filename, store, _parent);
      if (dp) return dp;
    }
  }
  return nullptr;
}

QStringList DataProviderFactory::registeredImageFormats() {
  QStringList formats;
  foreach (int key, imageLoaders.uniqueKeys()) {
    foreach (auto loader, imageLoaders.values(key)) {
      formats += loader->fileFormatFilters();
    }
  }
  return formats;
}

bool DataProviderFactory::registerImageLoader(int priority, DataProvider::ImageFactoryClass* loader) {
  DataProviderFactory::getInstance().imageLoaders.insert(priority, loader);
  return true;
}

bool DataProviderFactory::registerDeviceOpener(int, DeviceOpener) {
return true;
}

