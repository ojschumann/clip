#include "projectionplane.h"
#include "ui_projectionplane.h"

ProjectionPlane::ProjectionPlane(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProjectionPlane)
{
    ui->setupUi(this);
}

ProjectionPlane::~ProjectionPlane()
{
    delete ui;
}
