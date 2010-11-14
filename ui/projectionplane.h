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
  void info(QString, int);
  void reflexInfo(int,int,int);
protected slots:
  void slotChangeMouseDragMode();
  void slotUpdateFPS();
  void resizeView();
  void slotLoadCrystalData();
  void slotOpenResolutionCalc();
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
    void on_imageToolboxAction_triggered();
    void on_flipVAction_triggered();
    void on_flipHAction_triggered();
    void on_rotCCWAction_triggered();
    void on_rotCWAction_triggered();
    void on_configAction_triggered();
    void on_openImgAction_triggered();
    void on_closeImgAction_triggered();
    void debugSlot();
};




#endif // PROJECTIONPLANE_H
