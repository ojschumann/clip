#ifndef RESOLUTIONCALCULATOR_H
#define RESOLUTIONCALCULATOR_H

#include <QWidget>
#include "tools/itemstore.h"

namespace Ui {
  class ResolutionCalculator;
}

class RulerModel;
class RulerItem;
class LaueImage;

class ResolutionCalculator : public QWidget
{
  Q_OBJECT

public:
  explicit ResolutionCalculator(ItemStore<RulerItem>& r, LaueImage* img, QWidget *parent = 0);
  virtual ~ResolutionCalculator();
public slots:
  void slotSelectionChanged();
  void slotCalcResolution();
protected slots:
  void deletePressed();
private:
  Ui::ResolutionCalculator *ui;
  ItemStore<RulerItem>& rulers;
  RulerModel* model;
  bool resolutionsLocked;
  LaueImage* image;
private slots:
    void on_pushButton_clicked();
    void on_cancelButton_clicked();
    void on_acceptButton_clicked();

};

#endif // RESOLUTIONCALCULATOR_H
