#ifndef MOUSEINFODISPLAY_H
#define MOUSEINFODISPLAY_H

#include <QWidget>

#include "tools/mousepositioninfo.h"

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
  Ui::MouseInfoDisplay *ui;
};

#endif // MOUSEINFODISPLAY_H
