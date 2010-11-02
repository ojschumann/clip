#ifndef RESOLUTIONCALCULATOR_H
#define RESOLUTIONCALCULATOR_H

#include <QWidget>


namespace Ui {
  class ResolutionCalculator;
}

class Projector;
class RulerModel;

class ResolutionCalculator : public QWidget
{
  Q_OBJECT

public:
  explicit ResolutionCalculator(Projector* p, QWidget *parent = 0);
  ~ResolutionCalculator();
public slots:
  void slotSelectionChanged();
  void slotCalcResolution();
private:
  Ui::ResolutionCalculator *ui;
  Projector* projector;
  RulerModel* model;
};

#endif // RESOLUTIONCALCULATOR_H
