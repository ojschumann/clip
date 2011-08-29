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

#ifndef BASDATAPROVIDER_H
#define BASDATAPROVIDER_H

#include <QMap>
#include <QVariant>
#include <QVector>

#include "image/dataprovider.h"


class BasDataProvider : public DataProvider
{
  Q_OBJECT
public:
  class Factory: public DataProvider::ImageFactoryClass {
  public:
    Factory() {}
    QStringList fileFormatFilters();
    DataProvider* getProvider(QString, ImageDataStore*, QObject* = 0);
  };

  virtual ~BasDataProvider();

  virtual const void* getData();
  virtual QSize size();
  virtual int bytesCount();
  virtual int pixelCount();
  virtual Format format();

  static const char Info_OriginalFilename[];
  static const char Info_IPSize[];
  static const char Info_XPixelSize[];
  static const char Info_YPixelSize[];
  static const char Info_BitsPerPixel[];
  static const char Info_Width[];
  static const char Info_Height[];
  static const char Info_Sensitivity[];
  static const char Info_Latitude[];
  static const char Info_ExposureDate[];
  static const char Info_UnixTime[];
  static const char Info_OverflowPixels[];
  static const char Info_Comment[];
  static const char Info_InfFilename[];
  static const char Info_InfPath[];
  static const char Info_PixelSize[];
  static const char INF_Suffix[];
  static const char IMG_Suffix[];

private:
  explicit BasDataProvider(QObject *parent = 0);
  BasDataProvider(const BasDataProvider&);
  BasDataProvider& operator=(const BasDataProvider&);
signals:

public slots:
private:
  QVector<float> pixelData;
  QSize dataSize;
};

#endif // BASDATAPROVIDER_H
