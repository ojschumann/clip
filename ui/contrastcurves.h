#ifndef CONTRASTCURVES_H
#define CONTRASTCURVES_H

#include <QWidget>
#include "core/projector.h"
#include "tools/signalingellipse.h"

class LaueImage;
class BezierCurve;
class ObjectStore;


namespace Ui {
    class ContrastCurves;
}

class ContrastCurves : public QWidget
{
    Q_OBJECT

public:
    explicit ContrastCurves(Projector* p, QWidget *parent = 0);
    ~ContrastCurves();
    void mousePressEvent(QMouseEvent *);
  public slots:
    void loadFromFile(const QString&);
    void saveToFile(const QString&);
    void newMarker(const QPointF&);
    void changeToCurve(int);
    void markerChanged();
    void updateCurveLines(int);
    void makeScales();
    void makeHScale();
    void makeVScale();
    void updateHistogram(QVector<int>, QVector<int>, QVector<int>);
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

    Ui::ContrastCurves *ui;
    QPointer<Projector> projector;
    QGraphicsScene scene;
    QList<BoundedEllipse*> handleMarkers;
    QList<QGraphicsPathItem*> curves;
    int activeCurve;
    QList<QGraphicsPathItem*> histogram;
};

#endif // CONTRASTCURVES_H
