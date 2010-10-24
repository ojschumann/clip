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

  on_newCrystal_triggered();
  on_newLaue_triggered();
  on_newStereo_triggered();


}

Clip::~Clip() {
  delete ui;
}


void Clip::on_newCrystal_triggered() {
  addMdiWindow(new CrystalDisplay(this));
}


void Clip::on_newLaue_triggered() {
  addProjector(new LauePlaneProjector(this));
}

void Clip::on_newStereo_triggered() {
  addProjector(new StereoProjector(this));
}

void Clip::addProjector(Projector* p) {
  ProjectionPlane* pp = new ProjectionPlane(connectToLastCrystal(p), this);
  connect(pp, SIGNAL(showConfig(QWidget*)), this, SLOT(addMdiWindow(QWidget*)));
  addMdiWindow(pp);
}

void Clip::addMdiWindow(QWidget* w) {
  QMdiSubWindow* m = ui->mdiArea->addSubWindow(w);
  m->setAttribute(Qt::WA_DeleteOnClose);
  m->show();
}

Projector* Clip::connectToLastCrystal(Projector *p) {
  QList<QMdiSubWindow*> mdiWindows = ui->mdiArea->subWindowList(QMdiArea::StackingOrder);
  while (!mdiWindows.empty()) {
    QMdiSubWindow* window = mdiWindows.takeLast();
    CrystalDisplay* cd = dynamic_cast<CrystalDisplay*>(window->widget());
    if (cd) {
      p->connectToCrystal(cd->getCrystal());
      break;
    }
  }
  return p;
}
