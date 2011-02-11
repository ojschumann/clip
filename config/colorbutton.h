#ifndef COLORBUTTON_H
#define COLORBUTTON_H

#include <QToolButton>

class ColorButton : public QToolButton
{
  Q_OBJECT
public:
  explicit ColorButton(const QColor& c, QWidget *parent = 0);

signals:
public slots:
  void setColor(const QColor& c);
};

#endif // COLORBUTTON_H
