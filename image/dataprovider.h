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

  class ImageFactoryClass {
  public:
    ImageFactoryClass() {}
    virtual QStringList fileFormatFilters()=0;
    virtual DataProvider* getProvider(QString, QObject* = 0)=0;
  };

  static DataProvider* loadImage(const QString&);
  static DataProvider* openDevice();
  virtual ~DataProvider();

  void insertFileInformation(const QString&);
  virtual void saveToXML(QDomElement);
  virtual void loadFromXML(QDomElement);
  virtual void loadNewData() {}
  virtual const void* getData()=0;
  virtual int bytesCount()=0;
  virtual int pixelCount()=0;
  virtual QSize size()=0;
  virtual QSizeF absoluteSize() { return QSizeF(); }
  virtual Format format()=0;
  virtual QString name();
  virtual QList<QWidget*> toolboxPages();
  virtual QVariant getProviderInfo(QString key) { return providerInformation[key]; }

protected:
  explicit DataProvider(QObject *parent = 0);
  QMap<QString, QVariant> providerInformation;

private:

signals:
  void newDataAvailable();
public slots:

};

#endif // DATAPROVIDER_H
