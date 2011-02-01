#ifndef BRUKERPROVIDER_H
#define BRUKERPROVIDER_H

#include <QVector>
#include "image/dataprovider.h"


class BrukerProvider: public DataProvider
{
  Q_OBJECT
public:
  class Factory: public DataProvider::ImageFactoryClass {
  public:
    Factory() {}
    QStringList fileFormatFilters();
    DataProvider* getProvider(QString, QObject* = 0);
  };

  virtual ~BrukerProvider();

  virtual const void* getData();
  virtual QSize size();
  virtual int bytesCount();
  virtual int pixelCount();
  virtual Format format();
  virtual QSizeF absoluteSize();
private:
  explicit BrukerProvider(QObject *parent = 0);
signals:

public slots:
private:
  QVector<unsigned int> pixelData;

};

#endif // BRUKERPROVIDER_H
