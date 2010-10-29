#ifndef RULERITEM_H
#define RULERITEM_H

#include <QGraphicsObject>
#include <QGraphicsEllipseItem>

class RulerItem : public QGraphicsObject
{
public:
  RulerItem(const QPointF&, const QPointF&, qreal, QGraphicsItem* parent=0);
  ~RulerItem();
  QRectF boundingRect() const;
  void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *);
  void setStart(const QPointF&);
  void setEnd(const QPointF&);
  QPointF getStart();
  QPointF getEnd();
  virtual QVariant itemChange(GraphicsItemChange, const QVariant &);
protected:
  QGraphicsEllipseItem* startHandle;
  QGraphicsEllipseItem* endHandle;
  qreal radius;
};

#endif // RULERITEM_H
