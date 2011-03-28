#ifndef IMAGEDATASTORE_H
#define IMAGEDATASTORE_H

#include <QObject>
#include <QMap>

class ImageDataStore : public QObject
{
  Q_OBJECT
public:
  enum DataType {
    PhysicalWidth,
    PhysicalHeight,
    Width,
    Height,
    AbsoluteWidth,
    AbsoluteHeight,
    PlaneDetectorToSampleDistance
  };
  explicit ImageDataStore(QObject *parent = 0);

  bool hasData(DataType d) const;
  double getData(DataType d) const;

public slots:
  void setData(DataType d, double v);

signals:
  void dataChanged(DataType, double);

private:
  QMap<DataType, double> dataStore;

};

#endif // IMAGEDATASTORE_H
