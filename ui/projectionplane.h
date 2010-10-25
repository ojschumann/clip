#ifndef PROJECTIONPLANE_H
#define PROJECTIONPLANE_H

#include <QWidget>
#include <QRubberBand>
#include <core/projector.h>
#include <QCustomEvent>
#include <tools/objectstore.h>
#include <QActionGroup>

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
signals:
  void showConfig(QWidget*);
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
  void mouseModeChanged(int);
protected slots:
  void slotChangeMouseMode(int);
  void slotUpdateFPS();
  void resizeView();
  void slotLoadCrystalData();
  //FIXME: Only for debuging
  void slotRandomRotation();
  void slotOpenProjectorConfig();
private:
  Ui::ProjectionPlane *ui;
  Projector* projector;
  QPointer<QWidget> projectorConfig;

  QPointF mousePressOrigin;
  QPointF lastMousePosition;
  QRubberBand* zoomRubber;

  QList<QRectF> zoomSteps;

  static QList<ProjectionPlane*> allPlanes;
  QActionGroup* mouseModeGroup;
};

#endif // PROJECTIONPLANE_H
