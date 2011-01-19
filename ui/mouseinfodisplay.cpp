#include "mouseinfodisplay.h"
#include "ui_mouseinfodisplay.h"

#include <QStyledItemDelegate>
#include <QPainter>
#include <iostream>

#include "ui/clip.h"
#include "core/crystal.h"
#include "core/projector.h"
#include "tools/indexparser.h"
#include "tools/tools.h"

using namespace std;


class NoBorderDelegate: public QStyledItemDelegate {
public:
  NoBorderDelegate(QTableWidget* parent=0): QStyledItemDelegate(parent) {
    // create grid pen
    int gridHint = parent->style()->styleHint(QStyle::SH_Table_GridLineColor, new QStyleOptionViewItemV4());
    QColor gridColor = static_cast<QRgb>(gridHint);
    _gridPen = QPen(gridColor, 0, parent->gridStyle());
  }
  virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {

    if (index.row()!=2 || index.column()>1) {
      QStyledItemDelegate::paint(painter, option, index);
      QPen oldPen = painter->pen();
      painter->setPen(_gridPen);

      painter->drawLine(option.rect.topRight(), option.rect.bottomRight());
      painter->drawLine(option.rect.bottomLeft(), option.rect.bottomRight());
      painter->setPen(oldPen);

    } else {
      painter->fillRect(option.rect, option.palette.background());
    }
  }
protected:
  QPen _gridPen;
};


MouseInfoDisplay::MouseInfoDisplay(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MouseInfoDisplay)
{
  ui->setupUi(this);
  //ui->angleTable->verticalHeader()->setDefaultSectionSize(fontMetrics().lineSpacing()+4);

  connect(ui->cursorBox, SIGNAL(toggled(bool)), ui->cursorTable, SLOT(setVisible(bool)));
  //TODO
  //connect(ui->cursorBox, SIGNAL(toggled(bool)), this, SLOT(resize()));

  ui->angleTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
  ui->angleTable->verticalHeader()->setResizeMode(QHeaderView::Stretch);
  ui->angleTable->horizontalHeader()->setMinimumSectionSize(fontMetrics().width("Negative")+8);
  ui->angleTable->horizontalHeader()->setStyleSheet("QHeaderView::Section { border-style: plain; border-width: 4px }");

  ui->scatterTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
  ui->scatterTable->verticalHeader()->setResizeMode(QHeaderView::Stretch);
  ui->scatterTable->horizontalHeader()->setMinimumSectionSize(fontMetrics().width("  123.456  "));
  ui->scatterTable->horizontalHeader()->setStyleSheet("QHeaderView::Section { border-style: plain; border-width: 4px }");

  ui->cursorTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
  ui->cursorTable->verticalHeader()->setResizeMode(QHeaderView::Stretch);
  ui->cursorTable->horizontalHeader()->setStyleSheet("QHeaderView::Section { border-style: plain; border-width: 4px }");

  ui->cursorTable->setItemDelegate(new NoBorderDelegate(ui->cursorTable));
}


MouseInfoDisplay::~MouseInfoDisplay()
{
  emit highlightMarker(Vec3D());
  delete ui;
}


class Macro {
public:
  Macro(QLineEdit* line, QString val, bool enabled) {
    if (enabled) {
      line->setText(val);
    } else {
      line->setText("");
    }
  }
  Macro(QTableWidgetItem* line, double val, bool enabled) {
    if (enabled) {
      line->setText(QString::number(val, 'f', 3));
    } else {
      line->setText("");
    }
  }
};

void MouseInfoDisplay::showMouseInfo(MousePositionInfo info) {

  Macro(ui->cursorTable->item(0,0), info.projectorPos.x(), info.valid);
  Macro(ui->cursorTable->item(1,0), info.projectorPos.y(), info.valid);

  Macro(ui->cursorTable->item(0,1), info.imagePos.x(), info.valid);
  Macro(ui->cursorTable->item(1,1), info.imagePos.y(), info.valid);

  for (int i=0; i<3; i++) {
    Macro(ui->cursorTable->item(i,2), info.normal(i), info.valid);
    Macro(ui->cursorTable->item(i,3), info.scattered(i), info.valid && info.scatteredOk);
  }

  if (info.nearestOk && !ui->lockReflection->isChecked()) {
    setPaletteForStatus(ui->reflex, true);
    // Can't set Text in displayReflection, as this is used from on_reflex_textEdited as well
    ui->reflex->setText(info.nearestReflection.toText());
    displayReflection(info.nearestReflection, info.detQMin, info.detQMax);
    emit highlightMarker(info.nearestReflection.hkl().toType<double>());
  }
}

void MouseInfoDisplay::displayReflection(const Reflection &r, double detQMin, double detQMax) {


  Macro(ui->scatterTable->item(0,0), r.d, true);
  Macro(ui->scatterTable->item(1,0), r.Qscatter, r.normal.x()>1e-6);
  Macro(ui->scatterTable->item(2,0), 180.0-180.0*M_1_PI*acos(r.scatteredRay.x()), r.normal.x()>1e-6);
  Vec3D n = r.normal;
  for (int i=0; i<3; i++) {
    ui->angleTable->item(i, 0)->setText(QString::number(180*M_1_PI*acos(qBound(-1.0, n(i), 1.0)), 'f', 2));
    ui->angleTable->item(i, 1)->setText(QString::number(180*M_1_PI*acos(qBound(-1.0,-n(i), 1.0)), 'f', 2));
  }
  QString diffOrders;
  foreach (int i, r.orders) {
    if ((2.0*detQMin<=i*r.Qscatter) && (i*r.Qscatter<=2.0*detQMax)) {
      if (!diffOrders.isEmpty()) diffOrders += ", ";
      diffOrders += QString::number(i);
    }
  }
  ui->diffOrders->setText(diffOrders);
}

void MouseInfoDisplay::on_reflex_textEdited(QString text) {
  IndexParser parser(text);
  setPaletteForStatus(ui->reflex, parser.isValid());
  if (parser.isValid()) {
    emit highlightMarker(parser.index());
  }
  Projector* p;
  if (parser.isValid() && ((p = Clip::getInstance()->getMostRecentProjector(true)))) {
    if (parser.isIntegral()) {
      Reflection r = p->getCrystal()->makeReflection(parser.index().toType<int>());
      displayReflection(r, p->Qmin(), p->Qmax());
    } else {
      Vec3D n = p->getCrystal()->hkl2Reziprocal(parser.index()).normalized();
      for (int i=0; i<3; i++) {
        ui->angleTable->item(i, 0)->setText(QString::number(180*M_1_PI*acos(qBound(-1.0, n(i), 1.0)), 'f', 2));
        ui->angleTable->item(i, 1)->setText(QString::number(180*M_1_PI*acos(qBound(-1.0,-n(i), 1.0)), 'f', 2));
      }
    }
  }
}

