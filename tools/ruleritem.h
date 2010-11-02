#ifndef RULERITEM_H
#define RULERITEM_H

#include <QGraphicsObject>
#include <QGraphicsEllipseItem>
#include <QPen>

class Projector;
class SignalingEllipseItem;


class RulerItem : public QGraphicsObject
{
  Q_OBJECT
public:
  RulerItem(const QPointF&, const QPointF&, Projector* p, QGraphicsItem* parent=0);
  ~RulerItem();
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
  SignalingEllipseItem* startHandle;
  SignalingEllipseItem* endHandle;
  bool highlighted;
  QPen pen;
  Projector* projector;
};

#endif // RULERITEM_H
