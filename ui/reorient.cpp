#include "reorient.h"
#include "ui_reorient.h"

#include "tools/indexparser.h"
#include "core/crystal.h"
#include "ui/clip.h"
#include "tools/tools.h"


Reorient::Reorient(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Reorient)
{
  ui->setupUi(this);

  gonioAxis << Vec3D() << Vec3D();
  gonioEdits << ui->axis1Edit << ui->axis2Edit;
  gonioCombos << ui->axis1Combo << ui->axis2Combo;
}

Reorient::~Reorient()
{
  delete ui;
}


Vec3D Reorient::fromNormal() {
  Crystal* c;
  if (!fromIndex.isNull() && (c = Clip::getInstance()->getMostRecentCrystal(true))) {
    if (ui->RezicheckBox->isChecked()) {
      return c->hkl2Reziprocal(fromIndex).normalized();
    } else {
      return c->uvw2Real(fromIndex).normalized();
    }
  }
  return Vec3D();
}

Vec3D Reorient::toNormal() {
  int index=ui->toCombo->currentIndex();
  if (index>2) {
    Crystal* c;
    if (!toIndex.isNull() && (c = Clip::getInstance()->getMostRecentCrystal(true))) {
      if (index==3) {
        return c->hkl2Reziprocal(fromIndex).normalized();
      } else {
        return c->uvw2Real(fromIndex).normalized();
      }
    }
  } else {
    Vec3D normal;
    normal(index)=1;
    return normal;
  }
  return Vec3D();
}

void Reorient::updateDisplay() {
  double aPhi, aChi;
  if (calcRotationAngles(aPhi, aChi)) {
    ui->axis1Display->setText(QString::number(aPhi, 'f', 2));
    ui->axis2Display->setText(QString::number(aChi, 'f', 2));
  } else {
    ui->axis1Display->setText("-----");
    ui->axis2Display->setText("-----");
  }
}


bool Reorient::calcRotationAngles(double &angle1, double &angle2) {
  if (gonioAxis.at(0).isNull() || gonioAxis.at(1).isNull()) return false;
  Vec3D nfrom = fromNormal();
  if (nfrom.isNull()) return false;
  Vec3D nto = toNormal();
  if (nto.isNull()) return false;

  Vec3D v1, v2;
  if (!calcLine(nfrom, nto, v1, v2)) return false;


  double score = -1;
  foreach (Vec3D v, calcPossibleIntermediatePositions(v1, v2)) {
    double aChi=calcRotationAngle(nfrom, v, gonioAxis.at(0));
    double aPhi=calcRotationAngle(v, nto, gonioAxis.at(1));
    if ((score<0) || (score > (aChi*aChi+aPhi*aPhi))) {
      angle1 = aChi;
      angle2 = aPhi;
      score = (aChi*aChi+aPhi*aPhi);
    }
  }
  if (score<0) return false;
  return true;
}


bool Reorient::calcLine(const Vec3D& nfrom, const Vec3D& nto, Vec3D& r1, Vec3D& r2) {
  Vec3D u2 = gonioAxis.at(0) % gonioAxis.at(1);
  if (u2.norm()<1e-6) return false;
  u2.normalize();

  double pc = gonioAxis.at(0) * gonioAxis.at(1);
  double denom = 1.0-pc*pc;
  if (denom<1e-6) return false;

  double t1 = nfrom * gonioAxis.at(0);
  double t2 = nto   * gonioAxis.at(1);

  double lambda = (t2-pc*t1)/denom;
  double mu     = (t1-pc*t2)/denom;

  r1 = gonioAxis.at(0)*lambda + gonioAxis.at(1)*mu;
  r2 = u2;
  return true;
}

QList<Vec3D> Reorient::calcPossibleIntermediatePositions(const Vec3D& u1, const Vec3D& u2) {
  QList<Vec3D> r;
  double l=u1.norm_sq();
  if (l==1.0) {
    r << u1;
  } else if (l<1.0) {
    l = sqrt(1.0-l);
    r << u1+u1*l << u1-u2*l;
  }
  return r;
}

double Reorient::calcRotationAngle(const Vec3D& from, const Vec3D& to, const Vec3D &axis) {
  Vec3D v1(from-axis*(from*axis));
  v1.normalize();

  Vec3D v2(to-axis*(to*axis));
  v2.normalize();

  double angle = acos(min(1.0, max(-1.0, v1*v2)));
  Mat3D M(from, to, axis);
  if (M.det()<0) {
    angle*=-1;
  }
  return angle;

}

void Reorient::on_doFirstRatation_clicked()
{
  /*        c=self.searchCrystal()
        if c and self.axisVectors[0] and self.axisVectors[1]:
            fv=self.fromVect()
            tv=self.toVect()
            if fv and tv:
                res=calcAngles(fv, tv, self.axisVectors[1], self.axisVectors[0])
                if len(res)>0:
                    c.addRotation(Mat3D(self.axisVectors[0], res[0][1]))
                    self.updateDisplay()*/
}

void Reorient::on_doBothRotation_clicked()
{
  /*    def doRotationSlot(self):
        c=self.searchCrystal()
        if c and self.axisVectors[0] and self.axisVectors[1]:
            fv=self.fromVect()
            tv=self.toVect()
            if fv and tv:
                res=calcAngles(fv, tv, self.axisVectors[1], self.axisVectors[0])
                if len(res)>0:
                    for i in range(2):
                        c.addRotation(Mat3D(self.axisVectors[i], res[0][1-i]))
                    self.updateDisplay()*/
}

void Reorient::on_toCombo_currentIndexChanged(int index) {
  ui->toEdit->setEnabled(index>2);
  ui->toEdit->setVisible(index>2);
  updateDisplay();
}

void Reorient::on_fromEdit_textChanged(QString text) {
  IndexParser parser(text);
  fromIndex = parser.index();
  setPaletteForStatus(ui->fromEdit, parser.isValid() && !fromIndex.isNull());
  updateDisplay();
}

void Reorient::on_toEdit_textChanged(QString text) {
  IndexParser parser(text);
  toIndex = parser.index();
  setPaletteForStatus(ui->toEdit, parser.isValid() && !toIndex.isNull());
  updateDisplay();
}

void Reorient::on_axis1Combo_currentIndexChanged(int index) {
  gonioAxisSelection(0, index);
}

void Reorient::on_axis2Combo_currentIndexChanged(int index) {
  gonioAxisSelection(1, index);
}

void Reorient::gonioAxisSelection(int axis, int index) {
  gonioEdits.at(axis)->setEnabled(index>2);
  gonioEdits.at(axis)->setVisible(index>2);
  if (index<3) {
    if (gonioCombos.at(axis)->currentIndex()==index)
      gonioCombos.at(axis)->setCurrentIndex((index+1)%3);
    gonioAxis[axis] = Vec3D();
    gonioAxis[axis](index)=1;
    updateDisplay();
  } else {
    gonioAxisTextChanged(axis, gonioEdits.at(axis)->text());
  }

}

void Reorient::on_axis1Edit_textChanged(QString text) {
  gonioAxisTextChanged(0, text);
}

void Reorient::on_axis2Edit_textChanged(QString text) {
  gonioAxisTextChanged(1, text);
}

void Reorient::gonioAxisTextChanged(int axis, QString text) {
  IndexParser parser(text);
  gonioAxis[axis] = parser.index();
  setPaletteForStatus(gonioEdits[axis], parser.isValid() && !parser.index().isNull());
  updateDisplay();
}
