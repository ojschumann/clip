/**********************************************************************
  Copyright (C) 2010 Olaf J. Schumann

  This file is part of the Cologne Laue Indexation Program.
  For more information, see <http://clip.berlios.de>

  Clip is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  Clip is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see http://www.gnu.org/licenses/
  or write to the Free Software Foundation, Inc., 51 Franklin Street,
  Fifth Floor, Boston, MA 02110-1301, USA.
 **********************************************************************/

#include "mouseinfodisplay.h"
#include "ui_mouseinfodisplay.h"

#include <QStyledItemDelegate>
#include <QPainter>
#include <QEvent>
#include <QWindowStateChangeEvent>
 

#include "ui/clip.h"
//#include "core/crystal.h"
//#include "core/projector.h"
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

static const char headerStyleSheet[] = "QHeaderView::Section { border-style: plain; border-width: 4px }";

MouseInfoDisplay::MouseInfoDisplay(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MouseInfoDisplay),
    lastVector()
{
  ui->setupUi(this);


  connect(ui->cursorBox, SIGNAL(toggled(bool)), this, SLOT(cursorTableVisiblyToggled(bool)));

  connect(Clip::getInstance(), SIGNAL(mousePositionInfo(MousePositionInfo)), this, SLOT(showMouseInfo(MousePositionInfo)));
  connect(this, SIGNAL(highlightMarker(Vec3D)), Clip::getInstance(), SIGNAL(highlightMarker(Vec3D)));

  ui->angleTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
  ui->angleTable->verticalHeader()->setResizeMode(QHeaderView::Stretch);
  ui->angleTable->horizontalHeader()->setMinimumSectionSize(fontMetrics().width("Negative")+8);
  ui->angleTable->horizontalHeader()->setStyleSheet(headerStyleSheet);

  ui->scatterTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
  ui->scatterTable->verticalHeader()->setResizeMode(QHeaderView::Stretch);
  ui->scatterTable->horizontalHeader()->setMinimumSectionSize(fontMetrics().width("  123.456  "));
  ui->scatterTable->horizontalHeader()->setStyleSheet(headerStyleSheet);

  ui->cursorTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
  ui->cursorTable->verticalHeader()->setResizeMode(QHeaderView::Stretch);
  ui->cursorTable->horizontalHeader()->setStyleSheet(headerStyleSheet);

  ui->cursorTable->setItemDelegate(new NoBorderDelegate(ui->cursorTable));

  cursorTableVisiblyToggled(ui->cursorBox->isChecked());
}


MouseInfoDisplay::~MouseInfoDisplay()
{
  doEmitHighlightMarker(Vec3D(), true);
  delete ui;
}

QSize MouseInfoDisplay::sizeHint() const {
  return minimumSizeHint();
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
  if (((lastSender != info.projector) || lastSender.isNull()) && info.projector) {
    if (!lastSender.isNull())
      lastSender->disconnect(this);
    lastSender = info.projector;
    connect(info.projector, SIGNAL(spotHighlightChanged(Vec3D, QString)), this, SLOT(receiveSpotHightlight(Vec3D, QString)));
    if (ui->lockReflection->isChecked()) {
      IndexParser parser(ui->reflex->text());
      if (parser.isValid())
        emit highlightMarker(parser.index());
    }
  }

  if (ui->cursorTable->isVisible()) {
    Macro(ui->cursorTable->item(0,0), info.projectorPos.x(), info.valid);
    Macro(ui->cursorTable->item(1,0), info.projectorPos.y(), info.valid);

    Macro(ui->cursorTable->item(0,1), info.imagePos.x(), info.valid);
    Macro(ui->cursorTable->item(1,1), info.imagePos.y(), info.valid);

    for (int i=0; i<3; i++) {
      Macro(ui->cursorTable->item(i,2), info.normal(i), info.valid);
      Macro(ui->cursorTable->item(i,3), info.scattered(i), info.valid && info.scatteredOk);
    }
  }

  if (info.nearestOk && (!ui->lockReflection->isChecked() || info.lockOnNearest)) {
    setPaletteForStatus(ui->reflex, true);
    ui->reflex->setText(info.nearestReflection.toText());
    ui->lockReflection->setChecked(info.lockOnNearest);
    emit highlightMarker(info.nearestReflection.hkl().toType<double>());
  }

}

void MouseInfoDisplay::receiveSpotHightlight(Vec3D v, QString diffOrders) {
  double d = v.norm();
  v /= d;
  double TT = 180.0*M_1_PI*acos(qBound(-1.0, v(0), 1.0));
  Macro(ui->scatterTable->item(0,0), 2.0*M_PI*d/v(0), v(0)>1e-4);
  Macro(ui->scatterTable->item(1,0), 1.0/d, true);
  Macro(ui->scatterTable->item(2,0), 180.0-2.0*TT, v(0)>1e-4);


  for (int i=0; i<3; i++) {
    ui->angleTable->item(i, 0)->setText(QString::number(180*M_1_PI*acos(qBound(-1.0, v(i), 1.0)), 'f', 2));
    ui->angleTable->item(i, 1)->setText(QString::number(180*M_1_PI*acos(qBound(-1.0,-v(i), 1.0)), 'f', 2));
  }

  ui->diffOrders->setText(diffOrders);
}

/*void MouseInfoDisplay::displayReflection(const Reflection &r, double detQMin, double detQMax) {


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
*/

void MouseInfoDisplay::on_reflex_textEdited(QString text) {
  IndexParser parser(text);
  setPaletteForStatus(ui->reflex, parser.isValid());
  if (parser.isValid()) {
    doEmitHighlightMarker(parser.index());
  }
}

void MouseInfoDisplay::doEmitHighlightMarker(const Vec3D &v, bool force) {
  if ((v!=lastVector) || force) {
    lastVector = v;
    emit highlightMarker(v);
  }
}

void MouseInfoDisplay::cursorTableVisiblyToggled(bool b) {
  ui->cursorTable->setVisible(b);
  ui->cursorBox->updateGeometry();
  updateGeometry();
  if (parentWidget()) {
    parentWidget()->updateGeometry();
    parentWidget()->adjustSize();
  }
}

bool MouseInfoDisplay::eventFilter(QObject *o, QEvent *e) {
  if (e->type()==QEvent::WindowStateChange) {
    if (parentWidget() && (parentWidget()->windowState() & Qt::WindowMaximized)) {
      parentWidget()->showNormal();
    }
  }
  return QWidget::eventFilter(o, e);
}

void MouseInfoDisplay::changeEvent(QEvent* e) {
  QWidget::changeEvent(e);
  if (e->type()==QEvent::ParentChange) {
    parentNeedSizeConstrain = true;
    parent()->installEventFilter(this);
  }
  if (parentNeedSizeConstrain && parentWidget() && isVisible()) {
    cursorTableVisiblyToggled(ui->cursorBox->isChecked());
    parentNeedSizeConstrain = false;
  }
}
