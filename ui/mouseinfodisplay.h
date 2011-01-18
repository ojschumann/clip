#ifndef MOUSEINFODISPLAY_H
#define MOUSEINFODISPLAY_H

#include <QWidget>

#include "tools/mousepositioninfo.h"

class Reflection;

namespace Ui {
  class MouseInfoDisplay;
}

class MouseInfoDisplay : public QWidget
{
  Q_OBJECT

public:
  explicit MouseInfoDisplay(QWidget *parent = 0);
  virtual ~MouseInfoDisplay();
public slots:
  void showMouseInfo(MousePositionInfo);
private:
  void displayReflection(const Reflection& r, double detQMin, double detQMax);

  Ui::MouseInfoDisplay *ui;

private slots:
    void on_reflex_textEdited(QString );
};

#endif // MOUSEINFODISPLAY_H
