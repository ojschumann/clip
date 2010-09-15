#include "projectionplane.h"
#include "ui_projectionplane.h"

ProjectionPlane::ProjectionPlane(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProjectionPlane)
{
    ui->setupUi(this);

    ui->toolBar->addAction(QIcon(":/fileopen.png"), "Open", this, SLOT(slotLoadCrystalData()));
}

ProjectionPlane::~ProjectionPlane()
{
    delete ui;
}


void ProjectionPlane::resizeEvent(QResizeEvent *e) {
    QSize s=ui->viewFrame->size();
    QRect r(10,10, s.width()-20, s.height()-20);
    ui->view->setGeometry(r);

}
