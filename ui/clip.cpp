/**********************************************************************
  Copyright (C) 2010 Olaf J. Schumann

  This file is part of the Cologne Laue Indexation Program.
  For more information, see <http://clip.berlios.de>

  Clip is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  Clip is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see http://www.gnu.org/licenses/
  or write to the Free Software Foundation, Inc., 51 Franklin Street,
  Fifth Floor, Boston, MA 02110-1301, USA.
 **********************************************************************/


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
  addMdiWindow(crystalDisplay)->systemMenu()->addAction("Save as Default", crystalDisplay->getCrystal(), SLOT(saveParametersAsDefault()));
}


void Clip::on_newLaue_triggered() {
  addProjector(new LauePlaneProjector());
}

void Clip::on_newStereo_triggered() {
  addProjector(new StereoProjector());
}

ProjectionPlane* Clip::addProjector(Projector* p) {
  if (!p) return 0;
  ProjectionPlane* pp = new ProjectionPlane(connectToLastCrystal(p), this);
  connect(pp, SIGNAL(rotationFromProjector(double)), this, SIGNAL(projectorRotation(double)));
  connect(pp, SIGNAL(mousePositionInfo(MousePositionInfo)), this, SIGNAL(mousePositionInfo(MousePositionInfo)));
  connect(this, SIGNAL(highlightMarker(Vec3D)), p, SLOT(setSpotHighlighting(Vec3D)));
  addMdiWindow(pp)->systemMenu()->addAction("Save as Default", p, SLOT(saveParametersAsDefault()));
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

void Clip::on_actionAbout_triggered(bool) {
  QFile f(":/AboutMessage.txt");
  f.open(QIODevice::ReadOnly);
  QString message = QString(f.readAll());

  message.replace("</VER_PRODUCTVERSION_STR>", QString(VER_PRODUCTVERSION_STR));
  message.replace("</HG_REPRO_ID>", QString(HG_REPRO_ID));
  message.replace("</HG_REPRO_REV>", QString(HG_REPRO_REV));
  message.replace("</HG_REPRO_DATE>", QString(HG_REPRO_DATE));
  message.replace("</BUILD_DATE>", QString(BUILD_DATE));
  message.replace("</BUILD_TIME>", QString(BUILD_TIME));

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
  raiseOrCreateToolWindow<MouseInfoDisplay>();
}

void Clip::on_actionRotation_triggered() {
  raiseOrCreateToolWindow<RotateCrystal>();
}

void Clip::on_actionReorientation_triggered() {
  raiseOrCreateToolWindow<Reorient>();
}

template <class T> T* Clip::raiseOrCreateToolWindow() {
  foreach (QMdiSubWindow* mdi, ui->mdiArea->subWindowList()) {
    if (dynamic_cast<T*>(mdi->widget())) {
      mdi->raise();
      ui->mdiArea->setActiveSubWindow(mdi);
      return NULL;
    }
  }
  T* tool = new T();
  QMdiSubWindow* m = addMdiWindow(tool);
  m->setWindowFlags(m->windowFlags() & ~Qt::WindowMaximizeButtonHint);
  return tool;
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
  QString filename = ConfigStore::getInstance()->initialWorkspaceFile();
  QFileInfo fInfo(filename);
  if (!fInfo.isReadable())
    filename = ":/DefaultWorkspace.cws";
  loadWorkspaceFile(filename);
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
    addMdiWindow(crystalDisplay)->systemMenu()->addAction("Save as Default", crystalDisplay->getCrystal(), SLOT(saveParametersAsDefault()));
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
