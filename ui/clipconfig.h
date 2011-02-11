#ifndef CLIPCONFIG_H
#define CLIPCONFIG_H

#include <QMainWindow>
#include <QColor>
#include <QSignalMapper>
#include <QToolButton>

namespace Ui {
  class ClipConfig;
}

class ClipConfig : public QMainWindow
{
  Q_OBJECT

public:
  enum ColorTypes {
    Spotmarker = 0,
    ZoneMarkerLine = 1,
    ZoneMarkerBackground = 2,
    SpotIndicators = 3
  };
  explicit ClipConfig(QWidget *parent = 0);
  ~ClipConfig();
protected slots:
  void colorButtonPressed(int id);

private:
  class ColorConfigButton {
  public:
    ColorConfigButton(QString name, QColor defaultColor);
    QToolButton* button() const;
    QColor color() const;
    QString name() const;
    void setColor(const QColor& c);
  private:
    QToolButton* _button;
    QColor _color;
    QString _name;
  };

  static ClipConfig* getInstance();

  Ui::ClipConfig *ui;
  QSignalMapper colorButtonMapper;
  QList<ColorConfigButton> colorButtons;
};

#endif // CLIPCONFIG_H
