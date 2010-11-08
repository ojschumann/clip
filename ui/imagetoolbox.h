#ifndef IMAGETOOLBOX_H
#define IMAGETOOLBOX_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <tools/signalingellipse.h>

namespace Ui {
  class ImageToolbox;
}

class LaueImage;
class SignalingEllipse;
class BezierCurve;
class ObjectStore;

class ImageToolbox : public QMainWindow
{
  Q_OBJECT

public:
  explicit ImageToolbox(LaueImage* img, QWidget *parent = 0);
  ~ImageToolbox();
  void resizeEvent(QResizeEvent *);
  void mousePressEvent(QMouseEvent *);
public slots:
  void newMarker(int, const QPointF&);
  void changeToCurve(int);
  void markerChanged();
  void updateCurveLines(int);
private:
  class BoundedEllipse: public SignalingEllipseItem {
  public:
    BoundedEllipse(QGraphicsItem *parent=0);
    QVariant itemChange(GraphicsItemChange change, const QVariant & value);
  };

  Ui::ImageToolbox *ui;
  LaueImage* image;
  QGraphicsScene scene;
  QList<BezierCurve*> bezierCurves;
  QList<BoundedEllipse*> handleMarkers;
  QList<QGraphicsPathItem*> curves;
  int activeCurve;
};

#endif // IMAGETOOLBOX_H
