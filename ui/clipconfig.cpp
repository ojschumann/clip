#include "clipconfig.h"
#include "ui_clipconfig.h"

#include <QLabel>
#include <QSettings>
#include <QFormLayout>
#include <QColorDialog>
#include <QFileInfo>
#include <QFileDialog>
#include <iostream>

#include "config/colorbutton.h"
#include "config/configstore.h"

using namespace std;



ClipConfig::ClipConfig(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ClipConfig)
{
  ui->setupUi(this);

  ConfigStore* config = ConfigStore::getInstance();

  QFormLayout* leftLayout = new QFormLayout();
  QFormLayout* rightLayout = new QFormLayout();
  static_cast<QGridLayout*>(ui->colorFrame->layout())->addLayout(leftLayout, 0, 0);
  static_cast<QGridLayout*>(ui->colorFrame->layout())->addLayout(rightLayout, 0, 1);

  for (int n=0; n<config->colorCount(); n++) {
    ColorButton* button = new ColorButton(config->color(n), this);
    ((2*n>config->colorCount())?rightLayout:leftLayout)->addRow(config->colorName(n), button);
    config->ensureColor(n, button, SLOT(setColor(QColor)));
    connect(button, SIGNAL(clicked()), &colorButtonMapper, SLOT(map()));
    colorButtonMapper.setMapping(button, n);
  }
  connect(&colorButtonMapper, SIGNAL(mapped(int)), this, SLOT(colorButtonPressed(int)));

  ui->zoneMarkerWidth->setValue(config->getZoneMarkerWidth());
  connect(ui->zoneMarkerWidth, SIGNAL(valueChanged(double)), config, SLOT(setZoneMarkerWidth(double)));

  ui->loadSize->setChecked(config->loadSizeFromWorkspace());
  connect(ui->loadSize, SIGNAL(toggled(bool)), config, SLOT(setLoadSizeFromWorkspace(bool)));

  ui->loadPosition->setChecked(config->loadPositionFromWorkspace());
  connect(ui->loadPosition, SIGNAL(toggled(bool)), config, SLOT(setLoadPositionFromWorkspace(bool)));

  ui->initailCWSFile->setText(config->initialWorkspaceFile());
  connect(ui->initailCWSFile, SIGNAL(textChanged(QString)), config, SLOT(setInitialWorkspaceFile(QString)));
}

ClipConfig::~ClipConfig()
{
  delete ui;
}


void ClipConfig::colorButtonPressed(int id) {
  ConfigStore* config = ConfigStore::getInstance();
  QColor initialColor = config->color(id);
  QColorDialog dialog(initialColor, this);
  dialog.setWindowTitle(QString("Select Color for %1").arg(config->colorName(id)));
  dialog.setOption(QColorDialog::ShowAlphaChannel);
  config->setColorChanger(id, &dialog, SIGNAL(currentColorChanged(QColor)));
  if (dialog.exec()==QDialog::Accepted) {
    config->setColor(id, dialog.currentColor());
  } else {
    config->setColor(id, initialColor);
  }
}

void ClipConfig::on_toolButton_clicked()
{
  QSettings settings;
  QString filename = QFileDialog::getOpenFileName(this, "Set Initial Workspace File", settings.value("LastDirectory").toString(),
                                                  "Clip Workspace Data (*.cws);;All Files (*)");
  QFileInfo finfo(filename);
  if (finfo.exists()) {
    ui->initailCWSFile->setText(filename);
  }
}
