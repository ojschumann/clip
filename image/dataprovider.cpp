/**********************************************************************
  Copyright (C) 2008-2011 Olaf J. Schumann

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

#include "dataprovider.h"
 
#include <QTableWidget>
#include <QHeaderView>
#include <QFileInfo>
#include <QDateTime>

#include "ui/resolutioncalculator.h"
#include "ui/contrastcurves.h"

using namespace std;

DataProvider::DataProvider(QObject* _parent) :
    QObject(_parent)
{
}

DataProvider::~DataProvider() {
}

DataProvider* DataProvider::openDevice() {
  return nullptr;
}

DataProvider* DataProvider::loadImage(const QString &) {
  return nullptr;
}

void DataProvider::insertFileInformation(const QString &filename) {
  QFileInfo info(filename);
  providerInformation.insert(Info_ImageFilename, info.fileName());
  providerInformation.insert(Info_ImagePath, info.canonicalFilePath());
  providerInformation.insert(Info_ImageCreationDate, info.created().toString(Qt::DefaultLocaleLongDate));
}

QString DataProvider::name() {
  if (providerInformation.contains(Info_ImageFilename)) {
    return providerInformation[Info_ImageFilename].toString();
  }
  return QString();
}

QList<QWidget*> DataProvider::toolboxPages() {
  QList<QWidget*> l;

  // ---------------- Info Table ---------------------------
  QList<QString> keys = providerInformation.keys();
  QTableWidget* table = new QTableWidget(keys.size(),2);
  for (int i=0; i<keys.size(); i++) {
    table->setItem(i, 0, new QTableWidgetItem(keys.at(i)));
    table->setItem(i, 1, new QTableWidgetItem(providerInformation[keys.at(i)].toString()));
    table->item(i,0)->setFlags(Qt::ItemIsEnabled);
    table->item(i,1)->setFlags(Qt::ItemIsEnabled);
  }
  table->setObjectName("Info");
  table->horizontalHeader()->setStretchLastSection(true);
  table->verticalHeader()->setDefaultSectionSize(table->fontMetrics().lineSpacing()+5);
  table->verticalHeader()->hide();
  table->horizontalHeader()->hide();
  l << table;
  // ---------------------------------------------------------
  return l;
}

void DataProvider::saveToXML(QDomElement) {}
void DataProvider::loadFromXML(QDomElement) {}

const char DataProvider::Info_ImageFilename[] = "ImgFilename";
const char DataProvider::Info_ImageSize[] = "Size";
const char DataProvider::Info_ImagePath[] = "Complete Path";
const char DataProvider::Info_ImageCreationDate[] = "File Creation Date";

