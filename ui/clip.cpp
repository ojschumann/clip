#include "clip.h"
#include "ui_clip.h"
#include "crystaldisplay.h"
#include <QMdiSubWindow>
#include <ui/projectionplane.h>
#include <core/laueplaneprojector.h>
#include <core/stereoprojector.h>

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
    Projector* p = new StereoProjector(this);
    QMdiSubWindow* w = ui->mdiArea->addSubWindow(new ProjectionPlane(p, this));
    w->show();
}
