#ifndef CLIP_H
#define CLIP_H

#include <QMainWindow>
#include <QSignalMapper>


class Projector;

namespace Ui {
  class Clip;
}

class Clip : public QMainWindow {
  Q_OBJECT

public:
  explicit Clip(QWidget *parent = 0);
  ~Clip();


public slots:
  // Menu Slots
  void on_newCrystal_triggered();
  void on_newLaue_triggered();
  void on_newStereo_triggered();
  void on_actionAbout_triggered(bool);
  void on_actionAbout_Qt_triggered(bool);

  // Slot for update of the Window-Submenu
  void slotUpdateWindowMenu();
  // Used by the Windows-Submenu
  void setActiveSubWindow(QWidget *window);
  void addMdiWindow(QWidget*);

protected:
  Projector* connectToLastCrystal(Projector*);
  void addProjector(Projector*);
  void addActions();
private:
  Ui::Clip *ui;

  QAction *closeAct;
  QAction *closeAllAct;
  QAction *tileAct;
  QAction *cascadeAct;
  QAction *nextAct;
  QAction *previousAct;
  QAction *separatorAct;
  QSignalMapper *windowMapper;
};

#endif // CLIP_H
