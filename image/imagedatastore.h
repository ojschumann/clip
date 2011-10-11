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

#ifndef IMAGEDATASTORE_H
#define IMAGEDATASTORE_H

#include <QObject>
#include <QMap>
#include <QTransform>
#include <QVariant>

#include "config.h"

class ImageDataStore : public QObject
{
  Q_OBJECT
public:
  enum DataType {
    PhysicalSize,
    PixelSize,
    PlaneDetectorToSampleDistance,
    CellA,
    CellB,
    CellC,
    CellAlpha,
    CellBeta,
    CellGamma,
  };
  explicit ImageDataStore(QObject* _parent = nullptr);

  bool hasData(DataType d) const;
  QVariant getData(DataType d) const;
  QSizeF getTransformedSizeData(DataType d) const;
  void setTransformedSizeData(DataType d, const QSizeF& s);

public slots:
  void setData(DataType d, QVariant v);
  void addTransform(const QTransform& t);

signals:
  void dataChanged(ImageDataStore::DataType, QVariant);
  void transformChanged();

private:
  QMap<DataType, QVariant> dataStore;
  QTransform imageTransform;

};

#endif // IMAGEDATASTORE_H
