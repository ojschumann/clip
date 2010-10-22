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

  windowMapper = new QSignalMapper(this);
  connect(windowMapper, SIGNAL(mapped(QWidget*)),
          this, SLOT(setActiveSubWindow(QWidget*)));

  addActions();
  connect(ui->menuWindows, SIGNAL(aboutToShow()), this, SLOT(slotUpdateWindowMenu()));



  on_actionNew_Crystal_triggered(true);
  on_actionNew_Laue_Projection_triggered(true);
  on_actionNew_Stereo_Projection_triggered(true);


}

Clip::~Clip(){
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


void Clip::on_actionNew_Laue_Projection_triggered(bool) {
  addMdiWindow(new ProjectionPlane(new LauePlaneProjector()));
}

void Clip::on_actionNew_Stereo_Projection_triggered(bool) {
  addMdiWindow(new ProjectionPlane(new StereoProjector()));
}

void Clip::on_actionAbout_triggered(bool) {
  QString message("This is the Cologne Laue Indexation Program (CLIP)\n");
  message += "a program to index and refine laue exposures.\n\n";
  message += "Version 4.0\n";
  message += "Mercurial info: $Revision$ $Date$\n\n";
  message += "Copyright (c) 2010 O.J.Schumann (o.j.schumann@gmail.com)\n";
  message += "Clip is licensed under the terms of the GNU General Public License.";


  QMessageBox::about(this, "Cologne Laue Indexation Program", message);
}

void Clip::on_actionAbout_Qt_triggered(bool) {
  QMessageBox::aboutQt(this, "Cologne Laue Indexation Program");
}

void Clip::slotUpdateWindowMenu() {
  ui->menuWindows->clear();
  ui->menuWindows->addAction(closeAct);
  ui->menuWindows->addAction(closeAllAct);
  ui->menuWindows->addSeparator();
  ui->menuWindows->addAction(tileAct);
  ui->menuWindows->addAction(cascadeAct);
  ui->menuWindows->addSeparator();
  ui->menuWindows->addAction(nextAct);
  ui->menuWindows->addAction(previousAct);
  ui->menuWindows->addAction(separatorAct);

  QList<QMdiSubWindow *> windows = ui->mdiArea->subWindowList();
  separatorAct->setVisible(!windows.isEmpty());

  for (int i = 0; i < windows.size(); ++i) {

    QString text;
    if (i < 9) {
      text = tr("&%1 %2").arg(i + 1)
             .arg(windows.at(i)->widget()->windowTitle());
    } else {
      text = tr("%1 %2").arg(i + 1)
             .arg(windows.at(i)->widget()->windowTitle());
    }
    QAction *action  = ui->menuWindows->addAction(text);
    action->setCheckable(true);
    action ->setChecked(windows.at(i) == ui->mdiArea->activeSubWindow());
    connect(action, SIGNAL(triggered()), windowMapper, SLOT(map()));
    windowMapper->setMapping(action, windows.at(i));
  }
}

void Clip::addActions() {
  closeAct = new QAction(tr("Cl&ose"), this);
  closeAct->setStatusTip(tr("Close the active window"));
  connect(closeAct, SIGNAL(triggered()),
          ui->mdiArea, SLOT(closeActiveSubWindow()));

  closeAllAct = new QAction(tr("Close &All"), this);
  closeAllAct->setStatusTip(tr("Close all the windows"));
  connect(closeAllAct, SIGNAL(triggered()),
          ui->mdiArea, SLOT(closeAllSubWindows()));

  tileAct = new QAction(tr("&Tile"), this);
  tileAct->setStatusTip(tr("Tile the windows"));
  connect(tileAct, SIGNAL(triggered()), ui->mdiArea, SLOT(tileSubWindows()));

  cascadeAct = new QAction(tr("&Cascade"), this);
  cascadeAct->setStatusTip(tr("Cascade the windows"));
  connect(cascadeAct, SIGNAL(triggered()), ui->mdiArea, SLOT(cascadeSubWindows()));

  nextAct = new QAction(tr("Ne&xt"), this);
  nextAct->setShortcuts(QKeySequence::NextChild);
  nextAct->setStatusTip(tr("Move the focus to the next window"));
  connect(nextAct, SIGNAL(triggered()),
          ui->mdiArea, SLOT(activateNextSubWindow()));

  previousAct = new QAction(tr("Pre&vious"), this);
  previousAct->setShortcuts(QKeySequence::PreviousChild);
  previousAct->setStatusTip(tr("Move the focus to the previous "
                               "window"));
  connect(previousAct, SIGNAL(triggered()),
          ui->mdiArea, SLOT(activatePreviousSubWindow()));

  separatorAct = new QAction(this);
  separatorAct->setSeparator(true);

}

void Clip::setActiveSubWindow(QWidget *window) {
     if (!window)
         return;
     ui->mdiArea->setActiveSubWindow(qobject_cast<QMdiSubWindow *>(window));
 }
