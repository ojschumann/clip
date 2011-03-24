#ifndef COLORTEXTITEM_H
#define COLORTEXTITEM_H

#include <QGraphicsTextItem>

class ColorTextItem: public QGraphicsTextItem {
  Q_OBJECT
public:
  ColorTextItem(): QGraphicsTextItem() {}
public slots:
  void setColor(QColor c) { setDefaultTextColor(c); }
};

#endif // COLORTEXTITEM_H
