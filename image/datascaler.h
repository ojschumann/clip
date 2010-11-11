#ifndef DATASCALER_H
#define DATASCALER_H

#include <QObject>
#include <QImage>
#include <image/dataprovider.h>

class DataScaler : public QObject
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

  explicit DataScaler(DataProvider* dp, QObject *parent = 0);
  QImage getImage(const QSize& size, const QRectF& from);
signals:

public slots:
protected:
  virtual void redrawCache()=0;
  DataProvider* provider;
  QImage cache;
  QRectF lastFrom;
};

#include "image/datascaler.cpp"

#endif // DATASCALER_H
