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
  typedef DataScaler*(*ScalerGenerator)(DataScaler::Format format);

  static DataScalerFactory& getInstance();
  static bool registerDataScaler(DataScaler::Format, ScalerGenerator);

  DataScaler* getScaler(DataProvider* dp);

private:
  explicit DataScalerFactory(QObject *parent = 0);
  DataScalerFactory(const DataProviderFactory&);
  ~DataScalerFactory();

  QMap<DataScaler::Format, ScalerGenerator> scalerGenerators;

};

#endif // DATASCALERFACTORY_H
