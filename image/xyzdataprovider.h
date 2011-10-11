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

#ifndef XYZDATAPROVIDER_H
#define XYZDATAPROVIDER_H

#include "image/dataprovider.h"

#include <QVector>


class XYZDataProvider : public DataProvider
{
  Q_OBJECT
public:
  class Factory: public DataProvider::ImageFactoryClass {
  public:
    Factory() {}
    QStringList fileFormatFilters();
    DataProvider* getProvider(QString, ImageDataStore*, QObject* = nullptr);
  };
  virtual ~XYZDataProvider();

  virtual void saveToXML(QDomElement);
  virtual void loadFromXML(QDomElement);
  virtual const void* getData();
  virtual QSize size();
  virtual int bytesCount();
  virtual int pixelCount();
  virtual Format format();
private:
  explicit XYZDataProvider(QObject* _parent = nullptr);
  XYZDataProvider(const XYZDataProvider&);
  XYZDataProvider& operator=(const XYZDataProvider&);
signals:

    public slots:
private:
  QVector<float> pixelData;
  int imgWidth;
  int imgHeight;
};


#endif // XYZDATAPROVIDER_H
