#ifndef ZONEITEM_H
#define ZONEITEM_H

#include "tools/propagatinggraphicsobject.h"
#include "tools/abstractmarkeritem.h"

#include <QPen>
#include <QDomElement>
#include "tools/vec3D.h"

class Projector;
class CircleItem;

class ZoneItem : public PropagatingGraphicsObject, public AbstractProjectorMarkerItem
{
  Q_OBJECT
public:
  ZoneItem(const QPointF&, const QPointF&, Projector* p, QGraphicsItem* parent=0);
  virtual ~ZoneItem();
  QRectF boundingRect() const;
  QPainterPath shape () const;
  void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *);
  void setStart(const QPointF&);
  void setEnd(const QPointF&);
  QPointF getStart();
  QPointF getEnd();
  void highlight(bool);
  virtual Vec3D getMarkerNormal() const;

  void saveToXML(QDomElement base);
  void loadFromXML(QDomElement base);
private slots:
  void updatePolygon();
  void updateOptimalZone();
signals:
  void positionChanged();
  void itemClicked();
protected:
  QList<QPolygonF> generatePolygon(const Vec3D&, const Vec3D&);
  QRectF imgRect;
  CircleItem* startHandle;
  CircleItem* endHandle;
  bool highlighted;
  QList<QPolygonF> zoneLines;
  QList<QPolygonF> zonePolys;
  Vec3D zoneNormal;
};

#endif // ZONEITEM_H
