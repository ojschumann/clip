#ifndef PROJECTIONPLANE_H
#define PROJECTIONPLANE_H

#include <QMainWindow>
#include <QRubberBand>
#include <QCustomEvent>
#include <tools/objectstore.h>
#include <QActionGroup>
#include <QPointer>

class Projector;

namespace Ui {
  class ProjectionPlane;
}

class ProjectionPlane : public QMainWindow
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
  void info(QString, int);
protected slots:
  void slotChangeMouseDragMode();
  void slotUpdateFPS();
  void resizeView();
  void slotLoadCrystalData();
  void slotLoadImage();
  void slotCloseImage();
  void slotOpenProjectorConfig();
  void slotOpenResolutionCalc();
protected:
  void setupToolbar();
  QRectF zoomSceneRect();

  Ui::ProjectionPlane *ui;
  Projector* projector;

  QPointer<QWidget> projectorConfig;
  QPointer<QWidget> resoluctionCalc;

  QPointF mousePressOrigin;
  QPointF lastMousePosition;
  QRubberBand* zoomRubber;

  QList<QRectF> zoomSteps;

  static QList<ProjectionPlane*> allPlanes;

  QAction* zoomAction;
  QAction* panAction;
  QAction* rotAction;
  QAction* rulerAction;
  QAction* markAction;
  QAction* infoAction;
  QAction* openImgAction;
  QAction* closeImgAction;
  QAction* flipHAction;
  QAction* flipVAction;
  QAction* rotCWAction;
  QAction* rotCCWAction;
  QAction* colorCurveAction;
  QAction* printAction;
  QAction* configAction;

  QList<QAction*> imageTools;
};




#endif // PROJECTIONPLANE_H
