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

void Clip::addMdiWindow(QWidget *o) {
  ProjectionPlane* p = dynamic_cast<ProjectionPlane*>(o);
  if (p) {
    QList<QMdiSubWindow*> mdiWindows = ui->mdiArea->subWindowList(QMdiArea::StackingOrder);
    while (!mdiWindows.empty()) {
      QMdiSubWindow* window = mdiWindows.takeLast();
      CrystalDisplay* cd = dynamic_cast<CrystalDisplay*>(window->widget());
      if (cd) {
        p->getProjector()->connectToCrystal(cd->getCrystal());
        break;
      }
    }
  }
  connect(o, SIGNAL(info(QString, int)), ui->statusBar, SLOT(showMessage(QString, int)));
  QMdiSubWindow* w = ui->mdiArea->addSubWindow(o);
  w->setAttribute(Qt::WA_DeleteOnClose);
  w->show();
}

void Clip::on_actionNew_Crystal_triggered(bool) {
  addMdiWindow(new CrystalDisplay());
}


void Clip::on_actionNew_Projection_triggered(bool) {
  addMdiWindow(new ProjectionPlane(new StereoProjector()));
}
