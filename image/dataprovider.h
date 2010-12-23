#ifndef DATAPROVIDER_H
#define DATAPROVIDER_H

#include <QObject>
#include <QSizeF>
#include <QMap>
#include <QVariant>
#include <QDomElement>

class DataScaler;

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

  static DataProvider* loadImage(const QString&);
  static DataProvider* openDevice();
  ~DataProvider();

  virtual void saveToXML(QDomElement)=0;
  virtual void loadFromXML(QDomElement)=0;
  virtual void loadNewData() {}
  virtual const void* getData()=0;
  virtual int width()=0;
  virtual int height()=0;
  virtual int bytesCount()=0;
  virtual int pixelCount()=0;
  virtual Format format()=0;
  virtual QString name();
  virtual QSizeF absoluteSize() { return QSizeF(); }
  virtual QList<QWidget*> toolboxPages();

protected:
  explicit DataProvider(QObject *parent = 0);
  QMap<QString, QVariant> providerInformation;

private:

signals:
  void newDataAvailable();
public slots:

};

#endif // DATAPROVIDER_H
