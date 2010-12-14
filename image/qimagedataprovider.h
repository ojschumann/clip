#ifndef QIMAGEDATAPROVIDER_H
#define QIMAGEDATAPROVIDER_H

#include <QImage>
#include "image/dataprovider.h"

class QImageDataProvider : public DataProvider
{
  Q_OBJECT
public:
  static DataProvider* loadImage(const QString&, QObject*);

  virtual const void* getData();
  virtual int width();
  virtual int height();
  virtual int bytesCount();
  virtual int pixelCount();
  virtual Format format();
  QString name();
private:
  explicit QImageDataProvider(const QImage& img, QObject *parent = 0);
  ~QImageDataProvider();
  QImageDataProvider(const QImageDataProvider&) {}
  QImage data;

signals:

public slots:

};

#endif // QIMAGEDATAPROVIDER_H
