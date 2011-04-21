#ifndef IMAGEDATASTORE_H
#define IMAGEDATASTORE_H

#include <QObject>
#include <QMap>
#include <QTransform>
#include <QVariant>

class ImageDataStore : public QObject
{
  Q_OBJECT
public:
  enum DataType {
    PhysicalSize,
    PixelSize,
    PlaneDetectorToSampleDistance,
  };
  explicit ImageDataStore(QObject *parent = 0);

  bool hasData(DataType d) const;
  QVariant getData(DataType d) const;
  QSizeF getTransformedSizeData(DataType d) const;
  void setTransformedSizeData(DataType d, const QSizeF& s);

public slots:
  void setData(DataType d, QVariant v);
  void addTransform(const QTransform& t);

signals:
  void dataChanged(ImageDataStore::DataType, QVariant);
  void transformChanged();

private:
  QMap<DataType, QVariant> dataStore;
  QTransform imageTransform;

};

#endif // IMAGEDATASTORE_H
