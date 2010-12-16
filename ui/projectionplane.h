#ifndef PROJECTIONPLANE_H
#define PROJECTIONPLANE_H

#include <QMainWindow>
#include <QRubberBand>
#include <QCustomEvent>
#include <QActionGroup>
#include <QPointer>
#include "tools/objectstore.h"
#include "tools/mousepositioninfo.h"

class Projector;
class LaueImage;

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
  void info(QString, int);
  void reflexInfo(int,int,int);
  void mousePositionInfo(MousePositionInfo);
  void rotationFromProjector(double);
protected slots:
  void slotChangeMouseDragMode();
  void slotUpdateFPS();
  void resizeView();
  void slotLoadCrystalData();
  void slotOpenResolutionCalc();
  void generateMousePositionInfo(QPointF);
  void generateEmptyMousePositionInfo();
  void imageLoaded(LaueImage*);
  void imageClosed();
protected:
  void setupToolbar();
  QRectF zoomSceneRect();

  Ui::ProjectionPlane *ui;
  Projector* projector;

  QPointer<QWidget> projectorConfig;
  QPointer<QWidget> imageToolbox;

  QPointF mousePressOrigin;
  QPointF lastMousePosition;
  QRubberBand* zoomRubber;

  QList<QRectF> zoomSteps;

  static QList<ProjectionPlane*> allPlanes;

  QString lastImageOpenDir;

private slots:
  void on_actionCrop_triggered();
  void on_imageToolboxAction_triggered();
  void on_flipVAction_triggered();
  void on_flipHAction_triggered();
  void on_rotCCWAction_triggered();
  void on_rotCWAction_triggered();
  void on_configAction_triggered();
  void on_openImgAction_triggered();
  void on_closeImgAction_triggered();
};




#endif // PROJECTIONPLANE_H
