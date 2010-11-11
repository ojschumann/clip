#include "simplergbscaler.h"
#include "image/datascalerfactory.h"
#include <iostream>

using namespace std;

SimpleRGBScaler::SimpleRGBScaler(DataProvider* dp, QObject *parent) :
    DataScaler(dp, parent)
{
  cout << "Init SimpleRGBScaler" << endl;
}

SimpleRGBScaler::SimpleRGBScaler(const SimpleRGBScaler &): DataScaler(0)  {}

SimpleRGBScaler::~SimpleRGBScaler() {}

DataScaler* SimpleRGBScaler::getScaler(DataProvider *dp) {
  return new SimpleRGBScaler(dp);
}

void SimpleRGBScaler::redrawCache() {
  QImage tmp((uchar*)provider->getData(), provider->width(), provider->height(), QImage::Format_RGB32);

  int w = cache->width();
  int h = cache->height();

  double x0 = sourceRect.left()*tmp.width();
  double dx = sourceRect.width()*tmp.width()/w;

  double y0 = sourceRect.top()*tmp.height();
  double dy = sourceRect.height()*tmp.height()/h;

  QRgb* data = (QRgb*)cache->bits();

  double fy = y0;
  int liy = -1;
  for (int y=0; y<h; y++) {
    int iy = std::min(std::max(0, int(fy)), tmp.height()-1);
    fy += dy;
    if (liy==iy) {
      memcpy(data, data-w, cache->bytesPerLine());
      data += w;
    } else {
      QRgb* source = (QRgb*)tmp.scanLine(iy);
      double fx = x0;
      for (int x=0; x<w; x++) {
        int ix = std::min(std::max(0, int(fx)), tmp.width()-1);
        fx += dx;
        *data = *(source+ix);
        data++;
      }
    }
    liy = iy;
  }
}

bool SimpleRGBScalerRegistered = DataScalerFactory::registerDataScaler(DataProvider::RGB8Bit, &SimpleRGBScaler::getScaler);
