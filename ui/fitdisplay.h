#ifndef FITDISPLAY_H
#define FITDISPLAY_H

#include <QMainWindow>

class Crystal;

namespace Ui {
  class FitDisplay;
}

class FitDisplay : public QMainWindow
{
  Q_OBJECT

public:
  explicit FitDisplay(Crystal* c, QWidget *parent = 0);
  virtual ~FitDisplay();

private slots:

private:
  Ui::FitDisplay *ui;
  Crystal* crystal;
};



#endif // FITDISPLAY_H
