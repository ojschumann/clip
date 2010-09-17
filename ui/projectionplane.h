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
protected:
    QRectF zoomSceneRect();
    void resizeView();

private:
    Ui::ProjectionPlane *ui;
    Projector* projector;

    QPointF mousePressOrigin;
    QRubberBand* zoomRubber;

    QList<QRectF> zoomSteps;

};

#endif // PROJECTIONPLANE_H
