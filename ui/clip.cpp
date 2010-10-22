#include "clip.h"
#include "ui_clip.h"
#include <ui/crystaldisplay.h>
#include <QMdiSubWindow>
#include <ui/projectionplane.h>
#include <core/laueplaneprojector.h>
#include <core/stereoprojector.h>
#include <QMessageBox>

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

void Clip::on_actionAbout_triggered(bool) {
  QString message("This is the Cologne Laue Indexation Program (CLIP)\n");
  message += "a program to index and refine laue exposures.\n\n";
  message += "Version 4.0\n";
  message += "Mercurial revision: $Revision$ $Date$";
  message += "Copyright (c) 2010 O.J.Schumann (o.j.schumann@gmail.com)\n";
  message += "Clip is licensed under the terms of the GNU General Public License.";


  QMessageBox::about(this, "Cologne Laue Indexation Program", message);
}

void Clip::on_actionAbout_Qt_triggered(bool) {
  QMessageBox::aboutQt(this, "Cologne Laue Indexation Program");
}

