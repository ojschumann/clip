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




#ifndef MWDATAPROVIDER_H
#define MWDATAPROVIDER_H

#include <QMap>
#include <QVariant>
#include <QVector>

#include "image/dataprovider.h"


class MWDataProvider : public DataProvider
{
  Q_OBJECT
public:
  class Factory: public DataProvider::ImageFactoryClass {
  public:
    Factory() {}
    QStringList fileFormatFilters();
    DataProvider* getProvider(QString, ImageDataStore*, QObject* = nullptr);
  };

  virtual ~MWDataProvider();

  virtual const void* getData();
  virtual QSize size();
  virtual int bytesCount();
  virtual int pixelCount();
  virtual Format format();
private:
  explicit MWDataProvider(QObject* _parent = nullptr);
  MWDataProvider(const MWDataProvider&);
  MWDataProvider& operator=(const MWDataProvider&);
signals:

public slots:
private:
  QVector<float> pixelData;
};

#endif // MWDATAPROVIDER_H
