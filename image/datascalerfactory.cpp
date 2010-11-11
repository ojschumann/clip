#include "datascalerfactory.h"

DataScalerFactory::DataScalerFactory(QObject *parent) :
    QObject(parent)
{
}

DataScalerFactory::DataScalerFactory(const DataScalerFactory &) {};

DataScalerFactory::~DataScalerFactory() {}

DataScalerFactory& DataScalerFactory::getInstance() {
  static DataScalerFactory instance;
  return instance;
}

DataScaler* DataScalerFactory::getScaler(DataProvider* dp) {
  if (scalerGenerators.contains(dp->format())) {
    return (*scalerGenerators[dp->format()])(dp);
  }
  return NULL;
}

bool DataScalerFactory::registerDataScaler(DataScaler::Format format, ScalerGenerator generator) {
  DataScalerFactory::getInstance().scalerGenerators.insert(format, generator);
  return true;
}
