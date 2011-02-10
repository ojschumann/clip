#ifndef CLIPCONFIG_H
#define CLIPCONFIG_H

#include <QMainWindow>
#include <QColor>

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

private:
  Ui::ClipConfig *ui;
};

#endif // CLIPCONFIG_H
