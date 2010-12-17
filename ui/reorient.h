#ifndef REORIENT_H
#define REORIENT_H

#include <QWidget>
#include <QPair>

namespace Ui {
  class Reorient;
}

class Reorient : public QWidget
{
  Q_OBJECT

public:
  explicit Reorient(QWidget *parent = 0);
  ~Reorient();

  QPair<Vec3D, bool> fromNormal();
  QPair<Vec3D, bool> toNormal();

public slots:
  void updateDisplay();


private:
  Ui::Reorient *ui;

private slots:
    void on_toCombo_currentIndexChanged(int index);
    void on_doBothRotation_clicked();
    void on_doFirstRatation_clicked();
};

#endif // REORIENT_H
