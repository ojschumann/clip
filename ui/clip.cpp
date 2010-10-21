#include "clip.h"
#include "ui_clip.h"
#include <ui/crystaldisplay.h>
#include <QMdiSubWindow>
#include <ui/projectionplane.h>
#include <core/laueplaneprojector.h>
#include <core/stereoprojector.h>

Clip::Clip(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Clip)
{
    ui->setupUi(this);

    on_actionNew_Crystal_triggered(true);
    on_actionNew_Projection_triggered(true);


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
    //Projector* p = new LauePlaneProjector(this);
    QList<QMdiSubWindow*> mdiWindows = ui->mdiArea->subWindowList(QMdiArea::StackingOrder);
    while (!mdiWindows.empty()) {
      QMdiSubWindow* window = mdiWindows.takeLast();
      CrystalDisplay* cd = dynamic_cast<CrystalDisplay*>(window->widget());
      if (cd) {
        p->connectToCrystal(cd->getCrystal());
        break;
      }
    }

    QMdiSubWindow* w = ui->mdiArea->addSubWindow(new ProjectionPlane(p, this));
    w->show();
}
