#ifndef PROJECTIONPLANE_H
#define PROJECTIONPLANE_H

#include <QWidget>
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

protected:
    QRectF zoomSceneRect();
    void resizeView();

private:
    Ui::ProjectionPlane *ui;
    Projector* projector;

    QList<QRectF> zoomSteps;

};

#endif // PROJECTIONPLANE_H
