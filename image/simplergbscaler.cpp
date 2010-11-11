#include "simplergbscaler.h"
#include "image/datascalerfactory.h"

SimpleRGBScaler::SimpleRGBScaler(DataProvider* dp, QObject *parent) :
    DataScaler(dp, parent)
{
}


SimpleRGBScaler::SimpleRGBScaler(const SimpleRGBScaler &)  {}

SimpleRGBScaler::~SimpleRGBScaler() {}

DataScaler* SimpleRGBScaler::getScaler(DataProvider *) {
  return new SimpleRGBScaler(dp);
}

void SimpleRGBScaler::redrawCache() {
  QImage tmp((uchar*)provider->getData(), provider->width(), provider->height(), QImage::Format_RGB32);
}

bool SimpleRGBScalerRegistered = DataScalerFactory::registerDataScaler(DataScaler::RGB8Bit, &SimpleRGBScaler::getScaler);
