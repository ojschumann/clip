#include "tools.h"

#include <QWidget>
#include <QPalette>

void setPaletteForStatus(QWidget *widget, bool ok) {
  QPalette p = widget->palette();
  if (ok) {
    p.setColor(QPalette::Base, Qt::white);
  } else {
    p.setColor(QPalette::Base, QColor(255, 200, 200));
  }
  widget->setPalette(p);
}
