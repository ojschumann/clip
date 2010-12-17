#ifndef CLIP_H
#define CLIP_H

#include <QMainWindow>
#include <QSignalMapper>


class Projector;
class Crystal;

namespace Ui {
  class Clip;
}

class Clip : public QMainWindow {
  Q_OBJECT

public:
  static Clip* getInstance();
  static void clearInstance();

  Crystal* getMostRecentCrystal(bool checkProjectors=false);
private:
  explicit Clip(QWidget *parent = 0);
  Clip(const Clip&);
  ~Clip();
  static Clip* instance;
signals:
  void projectorRotation(double);
  void windowChanged();
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

private slots:
    void on_actionRotation_triggered();
    void on_actionReflection_Info_triggered();
};

#endif // CLIP_H
