#ifndef QIMAGEDATAPROVIDER_H
#define QIMAGEDATAPROVIDER_H

#include <QImage>
#include <QStringList>

#include "image/dataprovider.h"

class QImageDataProvider : public DataProvider
{
  Q_OBJECT
public:
  class Factory: public DataProvider::ImageFactoryClass {
  public:
    Factory() {}
    QStringList fileFormatFilters();
    DataProvider* getProvider(QString, ImageDataStore*, QObject* = 0);
  };

  virtual void saveToXML(QDomElement);
  virtual void loadFromXML(QDomElement);
  virtual const void* getData();
  virtual QSize size();
  virtual int bytesCount();
  virtual int pixelCount();
  virtual Format format();
private:
  explicit QImageDataProvider(const QImage& img, QObject *parent = 0);
  virtual ~QImageDataProvider();
  QImageDataProvider(const QImageDataProvider&) {}
  QImage data;

signals:

public slots:

};

#endif // QIMAGEDATAPROVIDER_H
