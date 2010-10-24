#ifndef CLIP_H
#define CLIP_H

#include <QMainWindow>
#include <core/projector.h>

namespace Ui {
  class Clip;
}

class Clip : public QMainWindow
{
  Q_OBJECT

public:
  explicit Clip(QWidget *parent = 0);
  ~Clip();

public slots:
  void on_newCrystal_triggered();
  void on_newLaue_triggered();
  void on_newStereo_triggered();
  void addMdiWindow(QWidget* w);

protected:
  Projector* connectToLastCrystal(Projector*);
  void addProjector(Projector*);
private:
  Ui::Clip *ui;
};

#endif // CLIP_H
