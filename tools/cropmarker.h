#ifndef CROPMARKER_H
#define CROPMARKER_H

#include <QGraphicsObject>

class SignalingEllipseItem;
class Projector;

class CropMarker : public QGraphicsObject
{
  Q_OBJECT
public:
  explicit CropMarker(Projector* p, const QPointF& pCenter, double _dx, double _dy, double _angle, QGraphicsItem  *parent = 0);
  QPolygonF getRect();
signals:
public slots:
  void promoteToRectangle() {};
protected slots:
  //void centralHandleMoved();
  //void outerHandleXMoved();
  //void outerHandleYMoved();
protected:
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
  QRectF boundingRect() const;
  QVariant itemChange(GraphicsItemChange change, const QVariant &value);

  //void setOuterHandlesFromData();

  QList<SignalingEllipseItem*> handles;
  Projector* projector;
};

#endif // CROPMARKER_H
