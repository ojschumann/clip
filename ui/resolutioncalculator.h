#ifndef RESOLUTIONCALCULATOR_H
#define RESOLUTIONCALCULATOR_H

#include <QWidget>


namespace Ui {
  class ResolutionCalculator;
}

class Projector;

class ResolutionCalculator : public QWidget
{
  Q_OBJECT

public:
  explicit ResolutionCalculator(Projector* p, QWidget *parent = 0);
  ~ResolutionCalculator();

public slots:
  void slotReloadModel();
private:
  Ui::ResolutionCalculator *ui;
  Projector* projector;
};

#endif // RESOLUTIONCALCULATOR_H
