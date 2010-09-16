#include "projectionplane.h"
#include "ui_projectionplane.h"

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
    QSizeF scaled = zoomSceneRect().size();
    scaled.scale(ui->viewFrame->size(), Qt::KeepAspectRatio);
    QRectF rect(QPointF(0,0), scaled);
    rect.moveCenter(ui->viewFrame->rect().center());
    ui->view->setGeometry(rect.toRect());
}

void ProjectionPlane::resizeEvent(QResizeEvent *e) {
    resizeView();
}
