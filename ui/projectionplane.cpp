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

    zoomRubber=new QRubberBand(QRubberBand::Rectangle, ui->view);
    resizeView();

}

ProjectionPlane::~ProjectionPlane()
{
    delete ui;
    delete zoomRubber;
}

QRectF ProjectionPlane::zoomSceneRect() {
    if (!zoomSteps.empty()) {
        return zoomSteps.last();
    }
    return projector->getScene()->sceneRect();
}

void ProjectionPlane::resizeView() {
    // Get ZoomRect
    QRectF minViewRect = zoomSceneRect();
    // Get its Size
    QSizeF minWidgetSize = minViewRect.size();
    // Scale that Size to the container size, keeping its AspectRatio
    minWidgetSize.scale(ui->viewFrame->size(), Qt::KeepAspectRatio);
    // It was scaled by that factor
    double scaleFactor = minWidgetSize.width()/minViewRect.width();
    // Scale the full Scene by that factor
    QSizeF maxWidgetSize = scaleFactor * projector->getScene()->sceneRect().size();
    // bound that to the widget and use as rectSize
    QRectF finalRect(QPointF(0,0), maxWidgetSize.boundedTo(ui->viewFrame->size()));
    finalRect.moveCenter(ui->viewFrame->rect().center());
    ui->view->setGeometry(finalRect.toRect());
    ui->view->fitInView(minViewRect);
}

void ProjectionPlane::resizeEvent(QResizeEvent *e) {
    resizeView();
}


void ProjectionPlane::mousePressEvent(QMouseEvent *e) {
    mousePressOrigin = ui->view->mapToScene(ui->view->mapFromGlobal(e->globalPos()));
    if (e->buttons()==Qt::LeftButton) {
        QRect r(QPoint(), QSize());
        zoomRubber->setGeometry(QRect());
        zoomRubber->show();
        //QPointF p = ui->view->mapToScene(ui->view->mapFromGlobal(e->globalPos()));
    } else if (e->buttons()==Qt::RightButton) {
        if (zoomSteps.size()>0)
            zoomSteps.removeLast();
        resizeView();
    }
}

void ProjectionPlane::mouseMoveEvent(QMouseEvent *e) {
    QPointF p = ui->view->mapToScene(ui->view->mapFromGlobal(e->globalPos()));
    if (e->buttons()==Qt::LeftButton) {
        zoomRubber->setGeometry(QRect(ui->view->mapFromScene(mousePressOrigin), ui->view->mapFromScene(p)).normalized());
    }
}

void ProjectionPlane::mouseReleaseEvent(QMouseEvent *e) {
    QPointF p = ui->view->mapToScene(ui->view->mapFromGlobal(e->globalPos()));
    if (e->button()==Qt::LeftButton) {
        zoomSteps.append(QRectF(mousePressOrigin, p).normalized());
        zoomRubber->hide();
        resizeView();
    }
    //ui->view->setDragMode(QGraphicsView::NoDrag);
}
