#include "histogramitem.h"

#include <QPainter>
#include <cmath>
#include <QTimer>
#include <iostream>

using namespace std;

HistogramItem::HistogramItem(QGraphicsItem *parent) :
    QGraphicsObject(parent)
{
  maxValue = 0;
  compMode = 0;
}


QRectF HistogramItem::boundingRect() const {
  return QRectF(0,0,1,1);
}

void HistogramItem::setCompMode(int c) {
  compMode=(c)%(33*8);
  cout << "compMode: " << compMode << endl;
  update();
}

void HistogramItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
  if (maxValue == 0 || pathes.empty()) return;
  painter->save();
  QList<QColor> l;
  l << QColor(0xFF, 0x60, 0x60);
  l << QColor(0x60, 0xFF, 0x60);
  l << QColor(0x60, 0x60, 0xFF);
  l << QColor(0xFF, 0xFF, 0x60);
  l << QColor(0x60, 0xFF, 0xFF);
  l << QColor(0xFF, 0x60, 0xFF);
  l << QColor(0x7F, 0x7F, 0x7F);

  painter->setPen(Qt::NoPen);

  painter->setTransform(QTransform::fromScale(1, 1.0/maxValue), true);
  foreach (QPainterPath path, pathes) {
    painter->setBrush(QBrush(l.takeFirst()));
    painter->drawPath(path);
  }
  painter->restore();
}

void HistogramItem::setHistogram(QVector<int> r, QVector<int> g, QVector<int> b) {
  QList<QVector<int> > l;
  l << r << g << b;

  pathes.clear();
  for (int i=0; i<7; i++) {
    pathes.append(QPainterPath());
    pathes[i].moveTo(0,0);
  }


  maxValue=0;
  for (int i=0; i<r.size(); i++) {
    double x = 1.0*i/(r.size()-1);
    double vals[3];
    for (int n=0; n<3; n++) {
      vals[n]=l.at(n).at(i);
      if (vals[n]>0) vals[n]=log(vals[n]);
      if (maxValue<vals[n]) maxValue=vals[n];
      pathes[n].lineTo(x, vals[n]);
    }
    for (int n=0; n<3; n++) {
      double val = std::min(vals[n], vals[(n+1)%3]);
      pathes[n+3].lineTo(x, val);
    }
    pathes[6].lineTo(x, std::min(vals[0], std::min(vals[1], vals[2])));
  }
  for (int i=0; i<pathes.size(); i++) {
    pathes[i].lineTo(1,0);
    pathes[i].lineTo(0,0);
  }

  update();
}
