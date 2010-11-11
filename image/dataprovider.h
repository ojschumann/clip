#ifndef DATAPROVIDER_H
#define DATAPROVIDER_H

#include <QObject>
#include <image/datascaler.h>

class DataProvider : public QObject
{
    Q_OBJECT
public:
  static DataProvider* loadImage(const QString&);
  static DataProvider* openDevice();

  virtual void loadNewData() {};
  virtual const void* getData()=0;
  virtual int width()=0;
  virtual int height()=0;
  virtual int bytesCount()=0;
  virtual int pixelCount()=0;
  virtual DataScaler::Format format()=0;

protected:
  explicit DataProvider(QObject *parent = 0);
private:

signals:
  void newDataAvailable();
public slots:

};

#endif // DATAPROVIDER_H
