#include "clip.h"
#include "ui_clip.h"
#include "crystaldisplay.h"
#include <QMdiSubWindow>
#include <ui/projectionplane.h>

Clip::Clip(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Clip)
{
    ui->setupUi(this);
}

Clip::~Clip()
{
    delete ui;
}


void Clip::on_actionNew_Crystal_triggered(bool) {
    QMdiSubWindow* w = ui->mdiArea->addSubWindow(new CrystalDisplay(this));
    w->show();
}


void Clip::on_actionNew_Projection_triggered(bool) {
    QMdiSubWindow* w = ui->mdiArea->addSubWindow(new ProjectionPlane(this));
    w->show();
}
