#include "tools.h"

#include <QWidget>
#include <QPalette>


void setPaletteForStatus(QWidget *widget, bool ok) {
  QPalette p = widget->palette();
  if (ok) {
    p.setColor(QPalette::Base, Qt::white);
  } else {
    p.setColor(QPalette::Base, QColor(255, 200, 200));
  }
  widget->setPalette(p);
}

QSizeF transformSize(const QSizeF& s, const QTransform& t) {
  // Map center and unit vectors
  QPointF c = t.map(QPointF(0,0));
  QPointF ex = t.map(QPointF(1,0));
  QPointF ey = t.map(QPointF(0,1));

  // Calculate new width and height
  double w = fasthypot((ex.x()-c.x())*s.width(), (ex.y()-c.y())*s.height());
  double h = fasthypot((ey.x()-c.x())*s.width(), (ey.y()-c.y())*s.height());

  return QSizeF(w, h);
}
