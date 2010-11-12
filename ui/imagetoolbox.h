#ifndef IMAGETOOLBOX_H
#define IMAGETOOLBOX_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <tools/signalingellipse.h>

namespace Ui {
  class ImageToolbox;
}

class Projector;
class LaueImage;
class SignalingEllipse;
class BezierCurve;
class ObjectStore;

class ImageToolbox : public QMainWindow
{
  Q_OBJECT

public:
  explicit ImageToolbox(Projector* p, QWidget *parent = 0);
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
  Projector* projector;
  QGraphicsScene scene;
  QList<BezierCurve*> bezierCurves;
  QList<BoundedEllipse*> handleMarkers;
  QList<QGraphicsPathItem*> curves;
  int activeCurve;

private slots:
    void on_doCrop_clicked();
    void on_actionCrop_triggered();
    void on_actionSave_Curve_triggered();
    void on_actionLoad_Curve_triggered();
};

#endif // IMAGETOOLBOX_H
