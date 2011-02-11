#include "colorbutton.h"

#include <QColorDialog>

ColorButton::ColorButton(const QColor& c, QWidget *parent) :
    QToolButton(parent)
{
  setColor(c);
}

void ColorButton::setColor(const QColor &c) {
  QPixmap pixmap(iconSize());
  pixmap.fill(c);
  setIcon(QIcon(pixmap));
}
