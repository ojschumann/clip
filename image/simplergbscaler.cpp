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

SimpleRGBScaler::~SimpleRGBScaler() {
  cout << "delete SimpleRGBScaler" << endl;
}

DataScaler* SimpleRGBScaler::getScaler(DataProvider *dp, QObject* parent) {
  return new SimpleRGBScaler(dp, parent);
}

#include <tools/debug.h>

void SimpleRGBScaler::redrawCache() {
  if (!cache) return;

  QImage tmp((uchar*)provider->getData(), provider->width(), provider->height(), QImage::Format_RGB32);

  QRgb* source = (QRgb*)tmp.bits();
  QRgb* data = (QRgb*)cache->bits();

  int w = cache->width();
  int h = cache->height();
  int sw = provider->width();
  int sh = provider->height();

  QPolygonF poly2(QRectF(0,0,w,h));
  QPolygonF poly3(sourceRect);
  poly2.pop_back();
  poly3.pop_back();

  QTransform out2sqare;
  QTransform::quadToQuad(poly2, poly3, out2sqare);
  QTransform t = out2sqare * sqareToRaw;

  for (int y=0; y<h; y++) {
    for (int x=0; x<w; x++) {
      QPointF s = t.map(QPointF(0.5+x, 0.5+y));
      int sx = int(s.x());
      int sy = int(s.y());
      if (sx<0 || sx>=sw || sy<0 || sy>=sh) {
        *data = 0xFFFF0000;
      } else {
        *data = *(source+sx+sw*sy);
      }
      data++;
    }
  }
}

bool SimpleRGBScalerRegistered = DataScalerFactory::registerDataScaler(DataProvider::RGB8Bit, &SimpleRGBScaler::getScaler);
