#ifndef XYZDATAPROVIDER_H
#define XYZDATAPROVIDER_H

#include "image/dataprovider.h"

#include <QVector>


class XYZDataProvider : public DataProvider
{
  Q_OBJECT
public:
  static DataProvider* loadImage(const QString&, QObject*);
  ~XYZDataProvider();

  virtual void saveToXML(QDomElement);
  virtual void loadFromXML(QDomElement);
  virtual const void* getData();
  virtual int width();
  virtual int height();
  virtual int bytesCount();
  virtual int pixelCount();
  virtual Format format();
private:
  explicit XYZDataProvider(QObject *parent = 0);
signals:

    public slots:
private:
  QVector<float> pixelData;
  int imgWidth;
  int imgHeight;
};


#endif // XYZDATAPROVIDER_H
