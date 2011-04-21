#include "resolutioncalculator.h"
#include "ui_resolutioncalculator.h"

#include <cmath>
#include <QShortcut>
#include <QStyledItemDelegate>

#include <tools/rulermodel.h>
#include <QAbstractTableModel>
#include "tools/ruleritem.h"
#include "tools/itemstore.h"
#include "tools/numberedit.h"
#include "tools/mat3D.h"
#include "tools/vec3D.h"
#include "image/laueimage.h"


class NumberEditDelegate: public QStyledItemDelegate {
public:
  NumberEditDelegate(QObject* parent=0): QStyledItemDelegate(parent) {}
  virtual QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    NumberEdit* n = new NumberEdit(parent, true);
    n->setButtonSymbols(QAbstractSpinBox::NoButtons);
    n->setMinimum(0.0);
    n->setMaximum(1000.0);
    return n;
  }
};

ResolutionCalculator::ResolutionCalculator(ItemStore<RulerItem>& r, LaueImage* img, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ResolutionCalculator),
    rulers(r),
    resolutionsLocked(false),
    image(img)
{
  ui->setupUi(this);

  hRes = -1.0;
  vRes = -1.0;

  ui->rulerView->verticalHeader()->setDefaultSectionSize(ui->rulerView->fontMetrics().lineSpacing());
  ui->rulerView->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);

  model = new RulerModel(rulers, img);
  ui->rulerView->setModel(model);
  ui->rulerView->setItemDelegate(new NumberEditDelegate);

  connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(slotCalcResolution()));
  connect(image->data(), SIGNAL(transformChanged()), this, SLOT(slotCalcResolution()));
  connect(ui->rulerView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(slotSelectionChanged()));
  connect(&r, SIGNAL(itemClicked(int)), ui->rulerView, SLOT(selectRow(int)));
  QShortcut* deleteShortcut = new QShortcut(Qt::Key_Delete, ui->rulerView);
  connect(deleteShortcut, SIGNAL(activated()), this, SLOT(deletePressed()));

  slotCalcResolution();
}

ResolutionCalculator::~ResolutionCalculator()
{
  delete ui;
  delete model;
}

void ResolutionCalculator::slotSelectionChanged() {
  QItemSelectionModel* selection = ui->rulerView->selectionModel();
  for (int n=0; n<rulers.size(); n++) {
    rulers.at(n)->highlight(selection->isRowSelected(n, QModelIndex()));
  }
}

void ResolutionCalculator::deletePressed() {
  int idx=ui->rulerView->currentIndex().row();
  rulers.del(idx);
}

void ResolutionCalculator::slotCalcResolution() {
  Mat3D M;
  Vec3D v;
  M.zero();
  M(2,2)=1;
  QSizeF s = image->data()->getTransformedSizeData(ImageDataStore::PixelSize);

  for (int n=0; n<rulers.size(); n++) {
    bool ok;
    double l = rulers.at(n)->data(0).toDouble(&ok);
    if (ok && (l>0.0)) {
      RulerItem* r = dynamic_cast<RulerItem*>(rulers.at(n));
      double dx = (r->getStart().x()-r->getEnd().x())*s.width();
      double dy = (r->getStart().y()-r->getEnd().y())*s.height();
      M(0,0) += dx*dx*dx*dx;
      M(1,0) += dx*dx*dy*dy;
      M(1,1) += dy*dy*dy*dy;
      v(0) += l*l*dx*dx;
      v(1) += l*l*dy*dy;
    }
  }

  if (resolutionsLocked) {
    M(0, 0) += M(1, 1);
    M(1, 0) *= 2;
    M(1, 1) = M(0, 0);
    v(0) += v(1);
    v(1) = v(0);
  }
  M(0, 1) = M(1, 0);
  if (fabs(M.det())>1e-6) {
    v = M.inverse()*v;
    hRes = sqrt(fabs(v(0)));
    vRes = sqrt(fabs(v(1)));
    ui->HResDisplay->setText(QString::number(1.0/hRes, 'f', 2));
    ui->VResDisplay->setText(QString::number(1.0/vRes, 'f', 2));
  }  else {
    ui->HResDisplay->setText("");
    ui->VResDisplay->setText("");
    hRes = -1;
    vRes = -1;
  }
  model->setResolution(hRes, vRes);
}

void ResolutionCalculator::on_acceptButton_clicked()
{
  if ((hRes>0) && (vRes>0)) {
    QSizeF s = image->data()->getTransformedSizeData(ImageDataStore::PixelSize);
    s.rwidth() *= hRes;
    s.rheight() *= vRes;
    image->data()->setTransformedSizeData(ImageDataStore::PhysicalSize, s);
  }
  rulers.clear();
}

void ResolutionCalculator::on_cancelButton_clicked()
{
  rulers.clear();
}

void ResolutionCalculator::on_pushButton_clicked()
{
  resolutionsLocked = ! resolutionsLocked;
  ui->pushButton->setIcon(QIcon(resolutionsLocked ? ":/icons/icons/lock.png" : ":/icons/icons/unlock.png"));
  slotCalcResolution();
}
