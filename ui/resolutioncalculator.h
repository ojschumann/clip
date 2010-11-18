#ifndef RESOLUTIONCALCULATOR_H
#define RESOLUTIONCALCULATOR_H

#include <QWidget>
#include "tools/itemstore.h"
namespace Ui {
  class ResolutionCalculator;
}

class RulerModel;

class ResolutionCalculator : public QWidget
{
  Q_OBJECT

public:
  explicit ResolutionCalculator(ItemStore& r, QWidget *parent = 0);
  ~ResolutionCalculator();
public slots:
  void slotSelectionChanged();
  void slotCalcResolution();
private:
  Ui::ResolutionCalculator *ui;
  ItemStore& rulers;
  RulerModel* model;

private slots:
    void on_cancelButton_clicked();
    void on_acceptButton_clicked();
};

#endif // RESOLUTIONCALCULATOR_H
