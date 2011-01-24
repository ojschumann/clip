#ifndef ROTATECRYSTAL_H
#define ROTATECRYSTAL_H

#include <QWidget>
#include <QSignalMapper>

class Crystal;

namespace Ui {
  class RotateCrystal;
}

class RotateCrystal : public QWidget
{
  Q_OBJECT

public:
  explicit RotateCrystal(QWidget *parent = 0);
  virtual ~RotateCrystal();
  virtual QSize sizeHint() const;
public slots:
  void rotAxisChanged();
  void addRotationAngle(double);
  void resetSum();
private slots:
  void on_axisEdit_textEdited(QString );
  void on_axisChooser_currentIndexChanged(int index);
  void loadAxisFromCrystal();
  void addRotation(int);
  void windowChanged();

private:
  Ui::RotateCrystal *ui;
  double angleSum;
  QSignalMapper buttonMapper;
  Crystal* paramsLoadedFromCrystal;
  bool axisChangeFromEdit;
};

#endif // ROTATECRYSTAL_H
