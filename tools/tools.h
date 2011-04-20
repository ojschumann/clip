#ifndef TOOLS_H
#define TOOLS_H

class QWidget;
class QSizeF;

#include <QTransform>
#include <cmath>

static inline int ggt(int a, int b) {
  while (b) {
    int tb=b;
    b=a%b;
    a=tb;
  }
  if (a<0) return -a;
  return a;
}

static inline double fasthypot(double a, double b) {
  a = fabs(a);
  b = fabs(b);
  if (a>b) {
    b /= a;
    return a * sqrt(1.0+b*b);
  } else {
    a /= b;
    return b * sqrt(1.0+a*a);
  }
}

void setPaletteForStatus(QWidget* widget, bool ok);

QSizeF transformSize(const QSizeF& s, const QTransform& t);


#endif // TOOLS_H
