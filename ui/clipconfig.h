#ifndef CLIPCONFIG_H
#define CLIPCONFIG_H

#include <QMainWindow>
#include <QSignalMapper>
#include <QToolButton>

namespace Ui {
  class ClipConfig;
}

class ClipConfig : public QMainWindow
{
  Q_OBJECT

public:
  explicit ClipConfig(QWidget *parent = 0);
  virtual ~ClipConfig();

protected slots:
  void colorButtonPressed(int id);

private:
  Ui::ClipConfig *ui;
  QSignalMapper colorButtonMapper;

private slots:
    void on_toolButton_clicked();
};

#endif // CLIPCONFIG_H
