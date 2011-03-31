#ifndef PROJECTIONGRAPHICSVIEW_H
#define PROJECTIONGRAPHICSVIEW_H

#include <QGraphicsView>
#include <image/laueimage.h>
#include <QPointer>

class ProjectionGraphicsView : public QGraphicsView
{
  Q_OBJECT
public:
  explicit ProjectionGraphicsView(QWidget *parent = 0);

  void dragEnterEvent(QDragEnterEvent *);
  void mousePressEvent(QMouseEvent *e);
  void mouseMoveEvent(QMouseEvent *);
  void mouseReleaseEvent(QMouseEvent *);
  void leaveEvent(QEvent *);
signals:
  void mouseMoved(QPointF);
  void mouseLeft();
public slots:
  void retakeMouseEvent() { viewIgnoresThisMouseEvent = false; }
  void setImage(LaueImage*);
protected:
  void drawBackground(QPainter *painter, const QRectF &rect);
  QPointer<LaueImage> image;

  bool viewIgnoresThisMouseEvent;

};

#endif // PROJECTIONGRAPHICSVIEW_H
