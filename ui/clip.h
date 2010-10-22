#ifndef CLIP_H
#define CLIP_H

#include <QMainWindow>
#include <QSignalMapper>

namespace Ui {
  class Clip;
}

class Clip : public QMainWindow
{
  Q_OBJECT

public:
  explicit Clip(QWidget *parent = 0);
  ~Clip();

  void addMdiWindow(QWidget*);
  void addActions();

public slots:
  void on_actionNew_Crystal_triggered(bool);
  void on_actionNew_Laue_Projection_triggered(bool);
  void on_actionNew_Stereo_Projection_triggered(bool);
  void on_actionAbout_triggered(bool);
  void on_actionAbout_Qt_triggered(bool);
  void slotUpdateWindowMenu();
  void setActiveSubWindow(QWidget *window);

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
