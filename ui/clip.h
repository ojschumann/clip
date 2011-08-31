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

#ifndef CLIP_H
#define CLIP_H

#include <QMainWindow>
#include <QSignalMapper>

#include "tools/mousepositioninfo.h"

class Projector;
class ProjectionPlane;
class Crystal;
class MousePositionInfo;
class QMdiSubWindow;

namespace Ui {
  class Clip;
}

class Clip : public QMainWindow {
  Q_OBJECT

public:
  static Clip* getInstance();
  static void clearInstance();

  Crystal* getMostRecentCrystal(bool checkProjectors=false);
  Projector* getMostRecentProjector(bool withCrystal=false);
private:
  explicit Clip(QWidget *parent = 0);
  Clip(const Clip&);
  virtual ~Clip();
  static Clip* instance;
signals:
  void projectorRotation(double);
  void windowChanged();
  void mousePositionInfo(MousePositionInfo);
  void highlightMarker(Vec3D);
public slots:
  // Menu Slots
  void on_newCrystal_triggered();
  void on_newLaue_triggered();
  void on_newStereo_triggered();
  void on_newDiffStereo_triggered();
  void on_actionAbout_triggered(bool);
  void on_actionAbout_Qt_triggered(bool);

  // Loads the initial Workspace (DefaultWorkspace.cws from ressource)
  void loadInitialWorkspace();
  bool loadWorkspaceFile(QString filename);

  // Slot for update of the Window-Submenu
  void slotUpdateWindowMenu();
  // Used by the Windows-Submenu
  void setActiveSubWindow(QWidget *window);
  QMdiSubWindow* addMdiWindow(QWidget*, bool deleteOnClose = true);
protected:
  Projector* connectToLastCrystal(Projector*);
  ProjectionPlane* addProjector(Projector*);
  void addActions();
private:
  template<class T> T* raiseOrCreateToolWindow();

  Ui::Clip *ui;

  QAction *closeAct;
  QAction *closeAllAct;
  QAction *tileAct;
  QAction *cascadeAct;
  QAction *nextAct;
  QAction *previousAct;
  QAction *separatorAct;
  QSignalMapper *windowMapper;

private slots:
    void on_actionConfiguration_triggered();
    void on_actionToggleMarkerEnabled_triggered();
    void on_actionToggleSpotsEnabled_triggered();
    void on_actionSave_Workspace_triggered();
    void on_actionOpen_Workspace_triggered();
    void on_actionReorientation_triggered();
    void on_actionRotation_triggered();
    void on_actionReflection_Info_triggered();
};

#endif // CLIP_H
