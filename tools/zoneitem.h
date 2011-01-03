#ifndef ZONEITEM_H
#define ZONEITEM_H

#include "tools/propagatinggraphicsobject.h"
#include "tools/abstractmarkeritem.h"

#include <QPen>
#include <QDomElement>
#include "tools/vec3D.h"

class Projector;
class CircleItem;
class GraphicsEllipseItem;

class ZoneItem : public PropagatingGraphicsObject, public AbstractProjectorMarkerItem
{
  Q_OBJECT
public:
  ZoneItem(const QPointF&, const QPointF&, Projector* p, QGraphicsItem* parent=0);
  ~ZoneItem();
  QRectF boundingRect() const;
  QPainterPath shape () const;
  void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *);
  void setStart(const QPointF&);
  void setEnd(const QPointF&);
  QPointF getStart();
  QPointF getEnd();
  void highlight(bool);
  bool isHighlighted();
  virtual Vec3D getMarkerNormal();

  void saveToXML(QDomElement base);
  void loadFromXML(QDomElement base);
private slots:
  void updatePolygon();
  void updateOptimalZone();
signals:
  void positionChanged();
protected:
  QRectF imgRect;
  QList<QPolygonF> generatePolygon(const Vec3D&, const Vec3D&);
  CircleItem* startHandle;
  CircleItem* endHandle;
  qreal radius;
  bool highlighted;
  QPen pen;
  QList<QPolygonF> zoneLines;
  QList<QPolygonF> zonePolys;
  Vec3D zoneNormal;
};

#endif // ZONEITEM_H
