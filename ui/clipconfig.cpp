#include "clipconfig.h"
#include "ui_clipconfig.h"

#include <QSettings>
#include <QLabel>
#include <QFormLayout>
#include <QAbstractTableModel>
#include <QPushButton>

class ColorModel: public QAbstractTableModel {
public:
  ColorModel(QObject* parent=0);
  int rowCount(const QModelIndex &parent) const;
  int columnCount(const QModelIndex &parent) const;
  QVariant data(const QModelIndex &index, int role) const;
private:
  QList<QColor> colors;
  QStringList colorNames;
};

ColorModel::ColorModel(QObject *parent): QAbstractTableModel(parent) {
  colorNames = QStringList() << "Spot Marker" << "Zone Marker Lines" << "Zone Marker Background" << "Spot Indicators";
  colors << QSettings().value(colorNames.at(ClipConfig::Spotmarker), QColor(0, 255, 0)).value<QColor>();
  colors << QSettings().value(colorNames.at(ClipConfig::ZoneMarkerLine), QColor(255, 255, 0)).value<QColor>();
  colors << QSettings().value(colorNames.at(ClipConfig::ZoneMarkerBackground), QColor(255, 0, 255)).value<QColor>();
  colors << QSettings().value(colorNames.at(ClipConfig::SpotIndicators), QColor(0, 255, 255)).value<QColor>();
}

int ColorModel::rowCount(const QModelIndex &parent) const {
  return colors.size();
}

int ColorModel::columnCount(const QModelIndex &parent) const {
  return 2;
}

QVariant ColorModel::data(const QModelIndex &index, int role) const {
  if ((role==Qt::DisplayRole) && (index.column()==0)) {
    return QVariant(colorNames.at(index.row()));
  } else if ((role==Qt::DecorationRole) && (index.column()==1)) {
    return QVariant(colors.at(index.row()));
  }
  return QVariant();
}

ClipConfig::ClipConfig(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ClipConfig)
{
  ui->setupUi(this);
  ui->tableView->setModel(new ColorModel);
  ui->tableView->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
  ui->tableView->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);

  QFormLayout* layout = new QFormLayout(ui->scrollArea->widget());

  QStringList colorNames = QStringList() << "Spot Marker" << "Zone Marker Lines" << "Zone Marker Background" << "Spot Indicators";
  foreach (QString s, colorNames) {
    layout->addRow(s, new QPushButton("test"));
  }
}

ClipConfig::~ClipConfig()
{
  delete ui;
}
