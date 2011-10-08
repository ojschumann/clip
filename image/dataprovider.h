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

#ifndef DATAPROVIDER_H
#define DATAPROVIDER_H

#include <QObject>
#include <QSizeF>
#include <QMap>
#include <QVariant>
#include <QDomElement>

class ImageDataStore;


class DataProvider : public QObject
{
    Q_OBJECT
public:
  enum Format {
    RGB8Bit,
    Float32,
    Float64,
    UInt8,
    UInt16,
    UInt32
  };

  class ImageFactoryClass {
  public:
    ImageFactoryClass() {}
    virtual QStringList fileFormatFilters()=0;
    virtual DataProvider* getProvider(QString, ImageDataStore*, QObject* = nullptr)=0;
  };

  static DataProvider* loadImage(const QString&);
  static DataProvider* openDevice();
  virtual ~DataProvider();

  void insertFileInformation(const QString&);
  virtual void saveToXML(QDomElement);
  virtual void loadFromXML(QDomElement);
  virtual void loadNewData() {}
  virtual const void* getData()=0;
  virtual int bytesCount()=0;
  virtual int pixelCount()=0;
  virtual QSize size()=0;
  virtual Format format()=0;
  virtual QString name();
  virtual QList<QWidget*> toolboxPages();
  virtual QVariant getProviderInfo(const QString& key) { return providerInformation[key]; }
  virtual QList<QString> getProviderInfoKeys() { return providerInformation.keys(); }

  static const char Info_ImageFilename[];
  static const char Info_ImageSize[];
  static const char Info_ImagePath[];
  static const char Info_ImageCreationDate[];


protected:
  explicit DataProvider(QObject* _parent = nullptr);
  QMap<QString, QVariant> providerInformation;

private:

signals:
  void newDataAvailable();
public slots:

};

#endif // DATAPROVIDER_H
