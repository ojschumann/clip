#include "simplergbscaler.h"
#include "image/datascalerfactory.h"
#include <iostream>

using namespace std;

SimpleRGBScaler::SimpleRGBScaler(DataProvider* dp, QObject *parent) :
    DataScaler(dp, parent)
{
  data = static_cast<QRgb const*>(dp->getData());
  datawidth = dp->width();
  dataheight = dp->height();
  cout << "Init SimpleRGBScaler" << endl;
}

SimpleRGBScaler::SimpleRGBScaler(const SimpleRGBScaler &): DataScaler(0)  {}

SimpleRGBScaler::~SimpleRGBScaler() {
  cout << "delete SimpleRGBScaler" << endl;
}

DataScaler* SimpleRGBScaler::getScaler(DataProvider *dp, QObject* parent) {
  return new SimpleRGBScaler(dp, parent);
}

QRgb SimpleRGBScaler::getRGB(const QPointF &p) {
  int x = static_cast<int>(p.x());
  int y = static_cast<int>(p.y());
  if (x<0 || x>=datawidth || y<0 || y>=dataheight) {
    return 0xFFFF0000;
  } else {
    return *(data+x+y*datawidth);
  }
}


bool SimpleRGBScalerRegistered = DataScalerFactory::registerDataScaler(DataProvider::RGB8Bit, &SimpleRGBScaler::getScaler);
