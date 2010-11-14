#ifndef RESOLUTIONCALCULATOR_H
#define RESOLUTIONCALCULATOR_H

#include <QWidget>
#include "core/projector.h"

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
  QPointer<Projector> projector;
  RulerModel* model;

private slots:
    void on_cancelButton_clicked();
    void on_acceptButton_clicked();
};

#endif // RESOLUTIONCALCULATOR_H
