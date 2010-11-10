#ifndef QIMAGEDATAPROVIDER_H
#define QIMAGEDATAPROVIDER_H

#include <QImage>
#include "image/dataprovider.h"

class QImageDataProvider : public DataProvider
{
  Q_OBJECT
public:
  static DataProvider* loadImage(const QString&);

  virtual const unsigned char* getData();
  virtual int width();
  virtual int height();
  virtual int bytesCount();
  virtual int pixelCount();

private:
  explicit QImageDataProvider(const QImage& img, QObject *parent = 0);
  QImageDataProvider(const QImageDataProvider&) {}
  QImage data;

signals:

public slots:

};

#endif // QIMAGEDATAPROVIDER_H
