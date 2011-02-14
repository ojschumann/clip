#include "clip.h"
#include "ui_clip.h"

#include <QMdiSubWindow>
#include <QMessageBox>
#include <QFileDialog>
#include <QSettings>
#include <QTimer>

#include "defs.h"
#include "core/projector.h"
#include "ui/projectionplane.h"
#include "core/laueplaneprojector.h"
#include "core/stereoprojector.h"
#include "ui/crystaldisplay.h"
#include "ui/mouseinfodisplay.h"
#include "ui/rotatecrystal.h"
#include "ui/reorient.h"
#include "core/crystal.h"
#include "core/projectorfactory.h"
#include "tools/xmllistiterators.h"
#include "ui/clipconfig.h"
#include "config/configstore.h"

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
  connect(ui->mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)), this, SIGNAL(windowChanged()));


  QTimer::singleShot(0, this, SLOT(loadInitialWorkspace()));
}


Clip::Clip(const Clip &) {
}


Clip::~Clip(){
  delete ui;
}

Clip* Clip::instance = 0;

Clip* Clip::getInstance() {
  if (!instance)
    instance = new Clip();
  return instance;
}

void Clip::clearInstance() {
  ConfigStore::clearInstance();
  if (instance) {
    delete instance;
    instance = 0;
  }
}



void Clip::on_newCrystal_triggered() {
  CrystalDisplay* crystalDisplay = new CrystalDisplay();
  addMdiWindow(crystalDisplay);
  crystalDisplay->loadDefault();
}


void Clip::on_newLaue_triggered() {
  addProjector(new LauePlaneProjector());
}

void Clip::on_newStereo_triggered() {
  addProjector(new StereoProjector())->loadDefault();
}

ProjectionPlane* Clip::addProjector(Projector* p) {
  if (!p) return 0;
  ProjectionPlane* pp = new ProjectionPlane(connectToLastCrystal(p), this);
  connect(pp, SIGNAL(rotationFromProjector(double)), this, SIGNAL(projectorRotation(double)));
  connect(pp, SIGNAL(mousePositionInfo(MousePositionInfo)), this, SIGNAL(mousePositionInfo(MousePositionInfo)));
  connect(this, SIGNAL(highlightMarker(Vec3D)), p, SLOT(setSpotHighlighting(Vec3D)));
  addMdiWindow(pp);
  return pp;
}

Projector* Clip::connectToLastCrystal(Projector *p) {
  Crystal* c = getMostRecentCrystal(false);
  if (c) p->connectToCrystal(c);
  return p;
}

Crystal* Clip::getMostRecentCrystal(bool checkProjectors) {
  QList<QMdiSubWindow*> mdiWindows = ui->mdiArea->subWindowList(QMdiArea::StackingOrder);
  while (!mdiWindows.empty()) {
    QMdiSubWindow* window = mdiWindows.takeLast();
    CrystalDisplay* cd;
    ProjectionPlane* pp;
    if ((cd=dynamic_cast<CrystalDisplay*>(window->widget()))!=NULL) {
      return cd->getCrystal();
    } else if (checkProjectors && ((pp=dynamic_cast<ProjectionPlane*>(window->widget()))!=NULL)) {
      Crystal* c = pp->getProjector()->getCrystal();
      if (c) return c;
    }
  }
  return NULL;
}

Projector* Clip::getMostRecentProjector(bool withCrystal) {
  QList<QMdiSubWindow*> mdiWindows = ui->mdiArea->subWindowList(QMdiArea::StackingOrder);
  while (!mdiWindows.empty()) {
    QMdiSubWindow* window = mdiWindows.takeLast();
    ProjectionPlane* pp;
    if (((pp=dynamic_cast<ProjectionPlane*>(window->widget()))!=NULL) && (!withCrystal || pp->getProjector()->getCrystal())) {
      return pp->getProjector();
    }
  }
  return NULL;
}

QMdiSubWindow* Clip::addMdiWindow(QWidget* w, bool deleteOnClose) {
  QMdiSubWindow* m = ui->mdiArea->addSubWindow(w);
  m->setAttribute(Qt::WA_DeleteOnClose, deleteOnClose);
  m->setWindowIcon(w->windowIcon());
  m->show();
  connect(w, SIGNAL(destroyed()), m, SLOT(deleteLater()));
  return m;
}

void Clip::setActiveSubWindow(QWidget *window) {
  if (!window)
    return;
  QMdiSubWindow* mdiWin= qobject_cast<QMdiSubWindow *>(window);
  if (mdiWin) {
    ui->mdiArea->setActiveSubWindow(mdiWin);
  } else {
    foreach (mdiWin, ui->mdiArea->subWindowList()) {
      if (mdiWin->widget()==window) {
        ui->mdiArea->setActiveSubWindow(mdiWin);
      }
    }
  }
}

#include "win_version.h"

void Clip::on_actionAbout_triggered(bool) {
  QString message("This is the Cologne Laue Indexation Program (CLIP)\n");
  message += "a program to index and refine laue exposures.\n\n";
  message += "Version " + QString(VER_PRODUCTVERSION_STR) + "\n";
  message += "Mercurial ID: " + QString(HG_REPRO_ID) + " Revision: " + HG_REPRO_REV + "\n";
  message += "Mercurial Date: " + QString(HG_REPRO_DATE) + "\n";
  message += "Build Date: " + QString(BUILD_DATE) +" " + QString(BUILD_TIME) + "\n\n";
  message += "Copyright (C) 2010 O.J.Schumann (o.j.schumann@gmail.com)\n";
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

  addAction(ui->actionToggleMarkerEnabled);
  addAction(ui->actionToggleSpotsEnabled);

}


void Clip::on_actionReflection_Info_triggered() {
  foreach (QMdiSubWindow* mdi, ui->mdiArea->subWindowList()) {
    if (dynamic_cast<MouseInfoDisplay*>(mdi->widget())) {
      mdi->raise();
      ui->mdiArea->setActiveSubWindow(mdi);
      return;
    }
  }
  MouseInfoDisplay* info = new MouseInfoDisplay();
  connect(this, SIGNAL(mousePositionInfo(MousePositionInfo)), info, SLOT(showMouseInfo(MousePositionInfo)));
  connect(info, SIGNAL(highlightMarker(Vec3D)), this, SIGNAL(highlightMarker(Vec3D)));
  QMdiSubWindow* m = addMdiWindow(info);
  m->setWindowFlags(m->windowFlags() & ~Qt::WindowMaximizeButtonHint);
  //m->show();
  //m->layout()->setSizeConstraint(QLayout::SetFixedSize);

}

void Clip::on_actionRotation_triggered()
{
  foreach (QMdiSubWindow* mdi, ui->mdiArea->subWindowList()) {
    if (dynamic_cast<RotateCrystal*>(mdi->widget())) {
      mdi->raise();
      ui->mdiArea->setActiveSubWindow(mdi);
      return;
    }
  }
  RotateCrystal* rotate = new RotateCrystal();
  connect(this, SIGNAL(projectorRotation(double)), rotate, SLOT(addRotationAngle(double)));
  addMdiWindow(rotate);
}

void Clip::on_actionReorientation_triggered()
{
  foreach (QMdiSubWindow* mdi, ui->mdiArea->subWindowList()) {
    if (dynamic_cast<Reorient*>(mdi->widget())) {
      mdi->raise();
      ui->mdiArea->setActiveSubWindow(mdi);
      return;
    }
  }
  Reorient* reorient = new Reorient();
  addMdiWindow(reorient);
}

void Clip::on_actionOpen_Workspace_triggered() {
  QSettings settings;
  QString filename = QFileDialog::getOpenFileName(this, "Load Workspace", settings.value("LastDirectory").toString(),
                                                  "Clip Workspace Data (*.cws);;All Files (*)");
  if (loadWorkspaceFile(filename))
    settings.setValue("LastDirectory", QFileInfo(filename).canonicalFilePath());
}

void Clip::loadInitialWorkspace() {
  // Needs to be a slot, because the windows are not placed correctly otherwise
  loadWorkspaceFile(":/DefaultWorkspace.cws");
}


const char XML_Clip_Workspace[] = "ClipWorkspace";
const char XML_Clip_CrystalConnection[] = "CrystalConnection";
const char XML_Clip_ConnectedCrystal[] = "ConnectedCrystal";
const char XML_Clip_ConnectedProjectors[] = "ConnectedProjectors";

bool Clip::loadWorkspaceFile(QString filename) {
  QDomDocument doc("ClipWorkspace");
  QFile file(filename);
  if (!file.open(QIODevice::ReadOnly))
    return false;
  if (!doc.setContent(&file)) {
    file.close();
    return false;
  }

  file.close();

  foreach (QDomElement crystalConnection, QDNLelements(doc.elementsByTagName(XML_Clip_CrystalConnection))) {
    QDomElement e = crystalConnection.elementsByTagName(XML_Clip_ConnectedCrystal).at(0).toElement();
    if (e.isNull()) continue;
    CrystalDisplay* crystalDisplay = new CrystalDisplay();
    addMdiWindow(crystalDisplay);
    crystalDisplay->loadFromXML(e);
    e = crystalConnection.elementsByTagName(XML_Clip_ConnectedProjectors).at(0).toElement();
    for (e=e.firstChildElement(); !e.isNull(); e=e.nextSiblingElement()) {
      ProjectionPlane* pp = addProjector(ProjectorFactory::getInstance().getProjector(e.attribute("projectortype")));
      if (pp) {
        pp->loadFromXML(e);
        pp->getProjector()->connectToCrystal(crystalDisplay->getCrystal());
      }
    }
  }
  return true;
}



void Clip::on_actionSave_Workspace_triggered() {
  QDomDocument doc(XML_Clip_Workspace);
  QDomElement docElement = doc.appendChild(doc.createElement(XML_Clip_Workspace)).toElement();
  foreach (QMdiSubWindow* mdi, ui->mdiArea->subWindowList()) {
    if (CrystalDisplay* cd = dynamic_cast<CrystalDisplay*>(mdi->widget())) {
      QDomElement connection = docElement.appendChild(doc.createElement(XML_Clip_CrystalConnection)).toElement();
      cd->saveToXML(connection.appendChild(doc.createElement(XML_Clip_ConnectedCrystal)).toElement());
      QDomElement connectedProjectors = connection.appendChild(doc.createElement(XML_Clip_ConnectedProjectors)).toElement();
      foreach (Projector* p, cd->getCrystal()->getConnectedProjectors()) {
        if (ProjectionPlane* pp = dynamic_cast<ProjectionPlane*>(p->parent())) {
          pp->saveToXML(connectedProjectors);
        }
      }

    }
  }

  QSettings settings;
  QString filename = QFileDialog::getSaveFileName(this, "Save Workspace", settings.value("LastDirectory").toString(),
                                                  "Clip Workspace Data (*.cws);;All Files (*)");

  QFile file(filename);
  if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    QTextStream ts(&file);
    doc.save(ts, 2);
    file.close();

    settings.setValue("LastDirectory", QFileInfo(filename).canonicalFilePath());
  }
}

void Clip::on_actionToggleSpotsEnabled_triggered() {
  Projector* p = getMostRecentProjector();
  if (p)
    p->enableSpots(!p->spotsEnabled());
}

void Clip::on_actionToggleMarkerEnabled_triggered() {
  Projector* p = getMostRecentProjector();
  if (p)
    p->enableMarkers(!p->markersEnabled());
}

void Clip::on_actionConfiguration_triggered() {
  addMdiWindow(new ClipConfig(this));
}
