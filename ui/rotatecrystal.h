#ifndef ROTATECRYSTAL_H
#define ROTATECRYSTAL_H

#include <QWidget>
#include <QSignalMapper>

namespace Ui {
  class RotateCrystal;
}

class RotateCrystal : public QWidget
{
  Q_OBJECT

public:
  explicit RotateCrystal(QWidget *parent = 0);
  ~RotateCrystal();
public slots:
  void rotAxisChanged();
  void addRotationAngle(double);
  void resetSum();
private slots:
  void loadAxisFromCrystal();
  void setRotationAxisToCrystal();
  void addRotation(int);

private:
  Ui::RotateCrystal *ui;
  double angleSum;
  QSignalMapper buttonMapper;
};

#endif // ROTATECRYSTAL_H
