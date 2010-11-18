#ifndef CROPMARKER_H
#define CROPMARKER_H

#include "tools/propagatinggraphicsobject.h"

class CircleItem;

class CropMarker : public PropagatingGraphicsObject
{
  Q_OBJECT
public:
  explicit CropMarker(const QPointF& pCenter, double _dx, double _dy, double _angle, double handleSize, QGraphicsItem  *parent = 0);
  QPolygonF getRect();
signals:
public slots:
  void promoteToRectangle() {};
  void setHandleSize(double);
protected slots:

protected:
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
  QRectF boundingRect() const;

  //void setOuterHandlesFromData();

  QList<CircleItem*> handles;
};

#endif // CROPMARKER_H
