#ifndef REORIENT_H
#define REORIENT_H

#include <QWidget>
#include <QPair>
#include <QLineEdit>
#include <QComboBox>

#include "tools/vec3D.h"

namespace Ui {
  class Reorient;
}

class Reorient : public QWidget
{
  Q_OBJECT

public:
  explicit Reorient(QWidget *parent = 0);
virtual ~Reorient();

public slots:
  void updateDisplay();

private:
  static Reorient* instance;
  void gonioAxisChanged();

  Vec3D fromNormal();
  Vec3D toNormal();

  bool calcRotationAngles(double& angle1, double& angle2);
  bool calcLine(const Vec3D& nfrom, const Vec3D& nto, Vec3D& r1, Vec3D& r2);
  QList<Vec3D> calcPossibleIntermediatePositions(const Vec3D& u1, const Vec3D& u2);
  double calcRotationAngle(const Vec3D& from, const Vec3D& to, const Vec3D& axis);
  Ui::Reorient *ui;

  Vec3D fromIndex;
  Vec3D toIndex;

  QList<Vec3D> gonioAxis;
  QList<QLineEdit*> gonioEdits;
  QList<QComboBox*> gonioCombos;

private slots:
  void on_axis2Edit_textChanged(QString );
  void on_axis1Edit_textChanged(QString );
  void on_axis2Combo_currentIndexChanged(int index);
  void on_axis1Combo_currentIndexChanged(int index);
  void on_toEdit_textChanged(QString );
  void on_fromEdit_textChanged(QString );
  void on_toCombo_currentIndexChanged(int index);
  void on_doBothRotation_clicked();
  void on_doFirstRatation_clicked();

  void gonioAxisSelection(int axis, int index);
  void gonioAxisTextChanged(int axis, QString text);
  void windowChanged();

};

#endif // REORIENT_H
