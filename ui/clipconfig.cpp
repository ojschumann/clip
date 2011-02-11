#include "clipconfig.h"
#include "ui_clipconfig.h"

#include <QSettings>
#include <QLabel>
#include <QFormLayout>
#include <QPushButton>
#include <QColorDialog>


ClipConfig::ColorConfigButton::ColorConfigButton(QString name, QColor defaultColor): _name(name) {
  QSettings settings;

  _button = new QToolButton;
  setColor(settings.value(name, defaultColor).value<QColor>());
}

QColor ClipConfig::ColorConfigButton::color() const {
  return _color;
}

QString ClipConfig::ColorConfigButton::name() const {
  return _name;
}

QToolButton* ClipConfig::ColorConfigButton::button() const {
  return _button;
}

void ClipConfig::ColorConfigButton::setColor(const QColor &c) {
  _color = c;
  QPixmap pixmap(_button->iconSize());
  pixmap.fill(_color);
  _button->setIcon(QIcon(pixmap));
}


ClipConfig::ClipConfig(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ClipConfig)
{
  ui->setupUi(this);

  QFormLayout* layout = new QFormLayout(ui->scrollArea->widget());

  colorButtons << ColorConfigButton("Spot Marker", QColor(255, 128, 0))
      << ColorConfigButton("Zone Marker Lines", QColor(0, 0, 0))
      << ColorConfigButton("Zone Marker background", QColor(255, 128, 0, 128))
      << ColorConfigButton("Spot Indicators", QColor(0, 255, 0));

  for (int n=0; n<colorButtons.size(); n++) {
    layout->addRow(colorButtons.at(n).name(), colorButtons.at(n).button());
    colorButtonMapper.setMapping(colorButtons.at(n).button(), n);
    connect(colorButtons.at(n).button(), SIGNAL(clicked()), &colorButtonMapper, SLOT(map()));
  }
  connect(&colorButtonMapper, SIGNAL(mapped(int)), this, SLOT(colorButtonPressed(int)));
}

ClipConfig* ClipConfig::getInstance() {
  static ClipConfig instance;
  return &instance;
}

void ClipConfig::colorButtonPressed(int id) {
  colorButtons[id].setColor(QColorDialog::getColor(colorButtons.at(id).color(),
                                                   this,
                                                   QString("Choose color for %1").arg(colorButtons.at(id).name()),
                                                   QColorDialog::ShowAlphaChannel));
}

ClipConfig::~ClipConfig()
{
  delete ui;
}
