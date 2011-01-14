#ifndef SPOTITEM_H
#define SPOTITEM_H

#include "tools/circleitem.h"
#include "tools/abstractmarkeritem.h"

class SpotItem : public CircleItem, public AbstractProjectorMarkerItem
{
  Q_OBJECT
public:
  explicit SpotItem(Projector* p, double r, QGraphicsItem* parent = 0);
  virtual Vec3D getMarkerNormal() const;
  virtual void highlight(bool b);

  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
  QRectF boundingRect() const;
  QPainterPath shape() const;

public slots:
  void slotSetMaxSearchIndex(int);
private slots:
  void slotInvalidateCache();
protected:
  bool isHighlighted;
};

#endif // SPOTITEM_H
