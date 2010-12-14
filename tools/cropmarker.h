#ifndef CROPMARKER_H
#define CROPMARKER_H

#include <QSignalMapper>

#include "tools/propagatinggraphicsobject.h"

class CircleItem;

class CropMarker : public QGraphicsObject
{
  Q_OBJECT
public:
  explicit CropMarker(const QPointF& pCenter, double _dx, double _dy, double _angle, double handleSize, QGraphicsItem  *parent = 0);
  QPolygonF getRect();
  virtual void setImgTransform(const QTransform &);

signals:
  void cancelCrop();
  void publishCrop(QPolygonF);
public slots:
  void promoteToRectangle() {};
  void setHandleSize(double);

protected slots:

protected:
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
  QRectF boundingRect() const;
  QPainterPath shape() const;

  void mousePressEvent(QGraphicsSceneMouseEvent *);
  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *);
  void mouseMoveEvent(QGraphicsSceneMouseEvent*);
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *);

  int pressedOnHandle;

  void positionHandles();
  void setCursors();

  QSizeF size;
  double handleSize;

  QList<QGraphicsRectItem*> handles;
};

#endif // CROPMARKER_H
