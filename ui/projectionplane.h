#ifndef PROJECTIONPLANE_H
#define PROJECTIONPLANE_H

#include <QWidget>
#include <QRubberBand>
#include <core/projector.h>


namespace Ui {
  class ProjectionPlane;
}

class ProjectionPlane : public QWidget
{
  Q_OBJECT

public:
  explicit ProjectionPlane(Projector*, QWidget *parent = 0);
  ~ProjectionPlane();
  virtual void resizeEvent(QResizeEvent *);
  virtual void mousePressEvent(QMouseEvent *);
  virtual void mouseMoveEvent(QMouseEvent *);
  virtual void mouseReleaseEvent(QMouseEvent *);

  virtual void dragEnterEvent(QDragEnterEvent *);
  virtual void dropEvent(QDropEvent *);
  Projector* getProjector() { return projector; }
protected:
  QRectF zoomSceneRect();
  enum MouseMode {
    MouseZoom,
    MousePan,
    MouseRotate
  };
  MouseMode mouseMode;
signals:
  void info(QString, int);
protected slots:
  void slotActivateZoom();
  void slotActivatePan();
  void slotActivateRotate();
  void slotUpdateFPS();
  void resizeView();
  void slotLoadCrystalData();
  //FIXME: Only for debuging
  void slotRandomRotation();

private:
  Ui::ProjectionPlane *ui;
  Projector* projector;

  QPointF mousePressOrigin;
  QPointF lastMousePosition;
  QRubberBand* zoomRubber;

  QList<QRectF> zoomSteps;

};

#endif // PROJECTIONPLANE_H
