#include "projectionplane.h"
#include "ui_projectionplane.h"

#include <QMouseEvent>
#include <QApplication>

ProjectionPlane::ProjectionPlane(Projector* p, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProjectionPlane)
{
    ui->setupUi(this);
    projector = p;
    projector->setParent(this); // Ensures, that projector is deleted at end

    ui->view->setScene(projector->getScene());


    ui->toolBar->addAction(style()->standardIcon(QStyle::SP_DialogOpenButton), "Open", this, SLOT(slotLoadCrystalData()));
}

ProjectionPlane::~ProjectionPlane()
{
    delete ui;
}

QRectF ProjectionPlane::zoomSceneRect() {
    if (!zoomSteps.empty()) {
        return zoomSteps.last();
    }
    return projector->getScene()->sceneRect();
}

void ProjectionPlane::resizeView() {
    QRectF minViewRect = zoomSceneRect();
    QSizeF scaled = minViewRect.size();
    scaled.scale(ui->viewFrame->size(), Qt::KeepAspectRatio);
    QRectF rect(QPointF(0,0), scaled);
    rect.moveCenter(ui->viewFrame->rect().center());
    ui->view->setGeometry(rect.toRect());
    ui->view->fitInView(minViewRect);
}

void ProjectionPlane::resizeEvent(QResizeEvent *e) {
    resizeView();
}


void ProjectionPlane::mousePressEvent(QMouseEvent *e) {
    //ui->view->setDragMode(QGraphicsView::RubberBandDrag);
    //QPointF p = ui->view->mapToScene(ui->view->mapFromGlobal(e->globalPos()));
}

void ProjectionPlane::mouseReleaseEvent(QMouseEvent *e) {
    //ui->view->setDragMode(QGraphicsView::NoDrag);
}
