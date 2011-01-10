#include "hkltool.h"
#include "ui_hkltool.h"

#include <QSortFilterProxyModel>
#include <QShortcut>

#include "indexing/livemarkermodel.h"
#include "ui/indexdisplay.h"
#include "ui/fitdisplay.h"

HKLTool::HKLTool(Crystal* c, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::HKLTool)
{
  ui->setupUi(this);


  ui->tabWidget->addTab(new IndexDisplay(c, ui->tabWidget), "Indexation");
  ui->tabWidget->addTab(new FitDisplay(c, ui->tabWidget), "Refinement");


  QSortFilterProxyModel* sorter = new QSortFilterProxyModel(this);
  markerModel = new LiveMarkerModel(c, this);
  sorter->setSourceModel(markerModel);
  sorter->setSortRole(Qt::UserRole);
  sorter->setDynamicSortFilter(true);
  ui->markerDisplay->setModel(sorter);
  ui->markerDisplay->sortByColumn(markerModel->columnCount()-1, Qt::AscendingOrder);
  connect(markerModel, SIGNAL(doHighlightMarker(int)), this, SLOT(highlightMarkerNr(int)));

  QShortcut* shortcut = new QShortcut(Qt::Key_Delete, ui->markerDisplay);
  connect(shortcut, SIGNAL(activated()), this, SLOT(deleteActiveMarker()));

  ui->markerDisplay->verticalHeader()->setDefaultSectionSize(fontMetrics().lineSpacing());
  ui->markerDisplay->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
  for (int n=markerModel->columnCount()-3; n<markerModel->columnCount(); n++) {
    ui->markerDisplay->horizontalHeader()->setResizeMode(n, QHeaderView::Stretch);
  }
  connect(ui->markerDisplay->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(highlightMarkers()));

}

HKLTool::~HKLTool() {
  delete ui;
}

void HKLTool::highlightMarkers() {
  QSortFilterProxyModel* sorter = dynamic_cast<QSortFilterProxyModel*>(ui->markerDisplay->model());
  QItemSelection selection = sorter->mapSelectionToSource(ui->markerDisplay->selectionModel()->selection());
  for (int i=0; i<markerModel->rowCount(); i++) {
    markerModel->highlightMarker(i, selection.contains(sorter->index(i, 0)));
  }
}

void HKLTool::highlightMarkerNr(int n) {
  QSortFilterProxyModel* sorter = dynamic_cast<QSortFilterProxyModel*>(ui->markerDisplay->model());
  ui->markerDisplay->setCurrentIndex(sorter->mapFromSource(markerModel->index(n, 0)));
}

void HKLTool::deleteActiveMarker() {
  QSortFilterProxyModel* sorter = dynamic_cast<QSortFilterProxyModel*>(ui->markerDisplay->model());
  QModelIndex idx = sorter->mapToSource(ui->markerDisplay->selectionModel()->currentIndex());
  if (idx.isValid())
    markerModel->deleteMarker(idx.row());
}
