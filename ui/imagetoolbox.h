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
  void mousePressEvent(QMouseEvent *);
public slots:
  void newMarker(const QPointF&);
  void changeToCurve(int);
  void markerChanged();
  void updateCurveLines(int);
  void makeScales();
  void makeHScale();
  void makeVScale();
private:
  class BoundedEllipse: public SignalingEllipseItem {
  public:
    BoundedEllipse(QGraphicsItem *parent=0);
    QVariant itemChange(GraphicsItemChange change, const QVariant & value);
    void setBBox(const QRectF& b) { bbox = b; };
    bool operator<(const BoundedEllipse& o);
  private:
    QRectF bbox;
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
