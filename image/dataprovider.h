#ifndef DATAPROVIDER_H
#define DATAPROVIDER_H

#include <QObject>

class DataProvider : public QObject
{
    Q_OBJECT
public:
  static DataProvider* loadImage(const QString&);
  static DataProvider* openDevice();

  virtual const unsigned char* getData()=0;
  virtual int width()=0;
  virtual int height()=0;
  virtual int bytesCount()=0;
  virtual int pixelCount()=0;

protected:
  explicit DataProvider(QObject *parent = 0);
private:

signals:

public slots:

};

#endif // DATAPROVIDER_H
