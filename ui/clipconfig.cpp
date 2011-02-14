#include "clipconfig.h"
#include "ui_clipconfig.h"

#include <QSettings>
#include <QLabel>
#include <QFormLayout>
#include <QColorDialog>
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

  QFormLayout* layout = new QFormLayout(ui->scrollArea->widget());

  for (int n=0; n<config->colorCount(); n++) {
    ColorButton* button = new ColorButton(config->color(n), this);
    layout->addRow(config->colorName(n), button);
    config->ensureColor(n, button, SLOT(setColor(QColor)));
    connect(button, SIGNAL(clicked()), &colorButtonMapper, SLOT(map()));
    colorButtonMapper.setMapping(button, n);
  }
  connect(&colorButtonMapper, SIGNAL(mapped(int)), this, SLOT(colorButtonPressed(int)));

  ui->zoneMarkerWidth->setValue(config->getZoneMarkerWidth());
  connect(ui->zoneMarkerWidth, SIGNAL(valueChanged(double)), config, SLOT(setZoneMarkerWidth(double)));
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
