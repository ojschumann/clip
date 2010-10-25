#ifndef PROJECTIONGRAPHICSVIEW_H
#define PROJECTIONGRAPHICSVIEW_H

#include <QGraphicsView>

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

protected:
  void paintEvent(QPaintEvent*);
  bool viewIgnoresThisMouseEvent;
  // TODO: Only for debugging!
  int frames;

};

#endif // PROJECTIONGRAPHICSVIEW_H
