#ifndef PROJECTIONPLANE_H
#define PROJECTIONPLANE_H

#include <QMainWindow>
#include <QRubberBand>
#include <QCustomEvent>
#include <QActionGroup>
#include <QPointer>
#include <QDomElement>

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
virtual ~ProjectionPlane();
  virtual void resizeEvent(QResizeEvent *);
  virtual void mousePressEvent(QMouseEvent *);
  virtual void mouseMoveEvent(QMouseEvent *);
  virtual void mouseReleaseEvent(QMouseEvent *);

  virtual void dragEnterEvent(QDragEnterEvent *);
  virtual void dropEvent(QDropEvent *);

  Projector* getProjector() { return projector; }

public slots:
  void saveToXML(QDomElement base);
  bool loadFromXML(QDomElement base);
  void loadDefault();


signals:
  void info(QString, int);
  void reflexInfo(int,int,int);
  void mousePositionInfo(MousePositionInfo);
  void rotationFromProjector(double);
protected slots:
  void slotChangeMouseDragMode();
  void resizeView();
  void slotOpenResolutionCalc();
  void generateMousePositionInfo(QPointF);
  void generateEmptyMousePositionInfo();
  void slotContextMenu();
  void slotContextSetRotationAxis();
  void slotContextSetRotationAxisOnSpot();
  void slotContextClearSpotMarkers();
  void slotContextClearZoneMarkers();
  void slotContextClearRulers();
  void slotContextClearAll();
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
  bool inMousePress;
  // Prevents adding of multiple ZoneMarkers or Rulers. Workaround :-(
  bool addedDragItemOnThisMove;

  QRubberBand* zoomRubber;
  QList<QRectF> zoomSteps;
  static QList<ProjectionPlane*> allPlanes;

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
