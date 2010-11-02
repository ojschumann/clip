#ifndef ZONEITEM_H
#define ZONEITEM_H

#include <QGraphicsObject>
#include <QGraphicsEllipseItem>
#include <QPen>

class Projector;
class SignalingEllipseItem;

class ZoneItem : public QGraphicsObject
{
  Q_OBJECT
public:
  ZoneItem(const QPointF&, const QPointF&, Projector* p, QGraphicsItem* parent=0);
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
private slots:
  void updatePolygon();
  void updateOptimalZone();
signals:
  void zoneChanged();
protected:
  QList<QPolygonF> generatePolygon(const Vec3D&);
  SignalingEllipseItem* startHandle;
  SignalingEllipseItem* endHandle;
  qreal radius;
  bool highlighted;
  QPen pen;
  Projector* projector;
  QList<QPolygonF> zoneSegments;
};

#endif // ZONEITEM_H
