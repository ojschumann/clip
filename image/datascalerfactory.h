#ifndef DATASCALERFACTORY_H
#define DATASCALERFACTORY_H

#include <QObject>
#include <QMap>
#include <image/datascaler.h>
#include <image/dataprovider.h>

class DataScalerFactory : public QObject
{
  Q_OBJECT
public:
  typedef DataScaler*(*ScalerGenerator)(DataProvider*, QObject*);

  static DataScalerFactory& getInstance();
  static bool registerDataScaler(DataProvider::Format, ScalerGenerator);

  DataScaler* getScaler(DataProvider* dp, QObject*);

private:
  explicit DataScalerFactory(QObject *parent = 0);
  DataScalerFactory(const DataScalerFactory&);
  ~DataScalerFactory();

  QMap<DataProvider::Format, ScalerGenerator> scalerGenerators;

};

#endif // DATASCALERFACTORY_H
