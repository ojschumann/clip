#include "rotatecrystal.h"
#include "ui_rotatecrystal.h"

#include "core/crystal.h"
#include "ui/clip.h"
#include "tools/indexparser.h"

RotateCrystal::RotateCrystal(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RotateCrystal)
{
  ui->setupUi(this);

  paramsLoadedFromCrystal=0;

  buttonMapper.setMapping(ui->but1Neg, 0);
  buttonMapper.setMapping(ui->but1Pos, 1);
  buttonMapper.setMapping(ui->but5Neg, 2);
  buttonMapper.setMapping(ui->but5Pos, 3);
  buttonMapper.setMapping(ui->butArbNeg, 4);
  buttonMapper.setMapping(ui->butArbPos, 5);

  connect(ui->but1Neg, SIGNAL(clicked()), &buttonMapper, SLOT(map()));
  connect(ui->but1Pos, SIGNAL(clicked()), &buttonMapper, SLOT(map()));
  connect(ui->but5Neg, SIGNAL(clicked()), &buttonMapper, SLOT(map()));
  connect(ui->but5Pos, SIGNAL(clicked()), &buttonMapper, SLOT(map()));
  connect(ui->butArbNeg, SIGNAL(clicked()), &buttonMapper, SLOT(map()));
  connect(ui->butArbPos, SIGNAL(clicked()), &buttonMapper, SLOT(map()));

  connect(&buttonMapper, SIGNAL(mapped(int)), this, SLOT(addRotation(int)));
  connect(ui->resetButton, SIGNAL(clicked()), this, SLOT(resetSum()));
  connect(Clip::getInstance(), SIGNAL(windowChanged()), this, SLOT(windowChanged()));

  windowChanged();
}

RotateCrystal::~RotateCrystal()
{
  delete ui;
}

void RotateCrystal::rotAxisChanged() {
  resetSum();
  loadAxisFromCrystal();
}

void RotateCrystal::addRotationAngle(double delta) {
  angleSum+=delta;
  // limit to 0...360
  angleSum -= 360.0*static_cast<int>(angleSum/360.0);
  if (angleSum<0.0) angleSum+=360;
  ui->angSumDisplay->setText(QString::number(angleSum, 'f', 2));
}

void RotateCrystal::resetSum() {
  addRotationAngle(-angleSum);
}

void RotateCrystal::addRotation(int id) {
  Crystal* c = Clip::getInstance()->getMostRecentCrystal(true);
  if (c) {
    double angleDefs[] = {-1, 1, -5, 5, -1, 1};
    double angle = angleDefs[id];
    if (id>3) angle *= ui->arbAngle->value();
    addRotationAngle(angle);
    c->addRotation(c->getLabSystemRotationAxis(), M_PI/180.0*angle);
  }
}

void RotateCrystal::loadAxisFromCrystal() {
  Crystal* c = Clip::getInstance()->getMostRecentCrystal(true);
  if (c) {
    paramsLoadedFromCrystal = c;
    if (c->getRotationAxisType()==Crystal::LabSystem) {
      Vec3D v(c->getRotationAxis());
      if (v==Vec3D(1, 0, 0)) {
        ui->axisChooser->setCurrentIndex(0);
      } else if (v==Vec3D(0, 1, 0)) {
        ui->axisChooser->setCurrentIndex(1);
      } else if (v==Vec3D(0, 0, 1)) {
        ui->axisChooser->setCurrentIndex(2);
      } else {
        ui->axisChooser->setCurrentIndex(3);
      }
    } else if (c->getRotationAxisType()==Crystal::ReziprocalSpace) {
      ui->axisChooser->setCurrentIndex(4);
    } else if (c->getRotationAxisType()==Crystal::DirectSpace) {
      ui->axisChooser->setCurrentIndex(5);
    }
    Vec3D v(c->getRotationAxis());
    ui->axisEdit->setText(IndexParser::formatIndex(v));
  }
}

void RotateCrystal::windowChanged() {
  if (paramsLoadedFromCrystal!=Clip::getInstance()->getMostRecentCrystal(true)) {
    if (paramsLoadedFromCrystal) paramsLoadedFromCrystal->disconnect(this, SLOT(rotAxisChanged()));
    rotAxisChanged();
    connect(paramsLoadedFromCrystal, SIGNAL(rotationAxisChanged()), this, SLOT(rotAxisChanged()), Qt::UniqueConnection);
  }
}

void RotateCrystal::on_axisChooser_currentIndexChanged(int index) {
  ui->axisEdit->setEnabled(index>=3);

  //Crystal* c = Clip::getInstance()->getMostRecentCrystal(true);
  if (Crystal* c = Clip::getInstance()->getMostRecentCrystal(true)) {
    if (index==5) {
      c->setRotationAxis(c->getRotationAxis(), Crystal::DirectSpace);
    } else if (index==4) {
      c->setRotationAxis(c->getRotationAxis(), Crystal::ReziprocalSpace);
    } else if (index==3) {
      c->setRotationAxis(c->getRotationAxis(), Crystal::LabSystem);
    } else {
      Vec3D axis;
      axis(index)=1;
      c->setRotationAxis(axis, Crystal::LabSystem);
      ui->axisEdit->setText(IndexParser::formatIndex(axis));
    }
  }
}



void RotateCrystal::on_axisEdit_textChanged(QString text)
{
  IndexParser parser(text);
  QPalette p = ui->axisEdit->palette();
  if (parser.isValid()) {
    p.setColor(QPalette::Base, Qt::white);
    if (Crystal* c = Clip::getInstance()->getMostRecentCrystal(true)) {
      c->setRotationAxis(parser.index(), c->getRotationAxisType());
    }
  } else {
    p.setColor(QPalette::Base, QColor(255, 200, 200));
  }
  ui->axisEdit->setPalette(p);
}

