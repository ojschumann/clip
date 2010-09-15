#ifndef PROJECTIONPLANE_H
#define PROJECTIONPLANE_H

#include <QWidget>

namespace Ui {
    class ProjectionPlane;
}

class ProjectionPlane : public QWidget
{
    Q_OBJECT

public:
    explicit ProjectionPlane(QWidget *parent = 0);
    ~ProjectionPlane();
    virtual void resizeEvent(QResizeEvent *);

private:
    Ui::ProjectionPlane *ui;
};

#endif // PROJECTIONPLANE_H
