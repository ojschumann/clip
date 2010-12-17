#ifndef RULERITEM_H
#define RULERITEM_H

#include "tools/propagatinggraphicsobject.h"
#include <QPen>

class CircleItem;

class RulerItem : public PropagatingGraphicsObject
{
  Q_OBJECT
public:
  RulerItem(const QPointF&, const QPointF&, double r, QGraphicsItem* parent=0);
  ~RulerItem();
  QRectF boundingRect() const;
  QPainterPath shape() const;
  void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *);
  void setStart(const QPointF&);
  void setEnd(const QPointF&);
  QPointF getStart();
  QPointF getEnd();
  void highlight(bool);
  bool isHighlighted();
public slots:
  void setHandleSize(double);
signals:
  void rulerChanged();
protected:
  CircleItem* startHandle;
  CircleItem* endHandle;
  bool highlighted;
  QPen pen;
  double radius;
};

#endif // RULERITEM_H
