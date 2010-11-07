#ifndef PROJECTIONGRAPHICSVIEW_H
#define PROJECTIONGRAPHICSVIEW_H

#include <QGraphicsView>
#include <tools/laueimage.h>
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
  int getFrames();
signals:

public slots:
  void retakeMouseEvent() { viewIgnoresThisMouseEvent = false; };
  void setImage(LaueImage*);
protected:
  void paintEvent(QPaintEvent*);
  void drawBackground(QPainter *painter, const QRectF &rect);
  QPointer<LaueImage> image;
  bool viewIgnoresThisMouseEvent;
  // TODO: Only for debugging!
  int frames;

};

#endif // PROJECTIONGRAPHICSVIEW_H
