#ifndef FITDISPLAY_H
#define FITDISPLAY_H

#include <QWidget>

class Crystal;
class FitObject;
class NelderMead;

namespace Ui {
  class FitDisplay;
}

class FitDisplay : public QWidget
{
  Q_OBJECT

public:
  explicit FitDisplay(Crystal* c, QWidget *parent = 0);
  virtual ~FitDisplay();

private slots:
  void fitObjectAdded(FitObject*);
  void fitObjectRemoved(FitObject*);
  void startStopFit();
  void toggleStartButtonText();
private:
  Ui::FitDisplay *ui;
  FitObject* mainFitObject;
  NelderMead* fitter;
};



#endif // FITDISPLAY_H
