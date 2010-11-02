#ifndef ZONEITEM_H
#define ZONEITEM_H

#include <QGraphicsObject>
#include <QGraphicsEllipseItem>
#include <QPen>

class Projector;

class ZoneItem : public QGraphicsObject
{
  Q_OBJECT
public:
  ZoneItem(const QPointF&, const QPointF&, qreal, Projector* p, QGraphicsItem* parent=0);
  ~ZoneItem();
  QRectF boundingRect() const;
  void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *);
  void setStart(const QPointF&);
  void setEnd(const QPointF&);
  QPointF getStart();
  QPointF getEnd();
  virtual QVariant itemChange(GraphicsItemChange, const QVariant &);
  void highlight(bool);
  bool isHighlighted();
signals:
  void rulerChanged();
protected:
  QGraphicsEllipseItem* startHandle;
  QGraphicsEllipseItem* endHandle;
  qreal radius;
  QPointF startPos;
  QPointF endPos;
  bool highlighted;
  QPen pen;
  Projector* projector;
};

#endif // ZONEITEM_H
