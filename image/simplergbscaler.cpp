#include "simplergbscaler.h"
#include "image/datascalerfactory.h"
#include <iostream>

using namespace std;

SimpleRGBScaler::SimpleRGBScaler(DataProvider* dp, QObject *parent) :
    DataScaler(dp, parent)
{
  data = static_cast<QRgb const*>(dp->getData());
  datawidth = dp->size().width();
  dataheight = dp->size().height();
}

SimpleRGBScaler::SimpleRGBScaler(const SimpleRGBScaler &): DataScaler(0)  {}

SimpleRGBScaler::~SimpleRGBScaler() {
}

DataScaler* SimpleRGBScaler::getScaler(DataProvider *dp, QObject* parent) {
  return new SimpleRGBScaler(dp, parent);
}
#include <cmath>
QRgb SimpleRGBScaler::getRGB(const QPointF &p) {
  int x = static_cast<int>(std::floor(p.x()));
  int y = static_cast<int>(std::floor(p.y()));
  if (x<0 || x>=datawidth || y<0 || y>=dataheight) {
    return 0xFFFF0000;
  } else {
    return *(data+x+y*datawidth);
  }
}


bool SimpleRGBScalerRegistered = DataScalerFactory::registerDataScaler(DataProvider::RGB8Bit, &SimpleRGBScaler::getScaler);
