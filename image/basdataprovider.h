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
private:
  explicit BasDataProvider(QObject *parent = 0);
signals:

public slots:
private:
  QVector<float> pixelData;
};

#endif // BASDATAPROVIDER_H
