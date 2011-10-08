/**********************************************************************
  Copyright (C) 2008-2011 Olaf J. Schumann

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

#include "hkltool.h"
#include "ui_hkltool.h"

#include <QSortFilterProxyModel>
#include <QShortcut>

#include "indexing/livemarkermodel.h"
#include "ui/indexdisplay.h"
#include "ui/fitdisplay.h"

HKLTool::HKLTool(Crystal* c, QWidget* _parent) :
    QMainWindow(_parent),
    ui(new Ui::HKLTool)
{
  ui->setupUi(this);

  // Setup Indexation and refinement tab
  IndexDisplay* indexDisplay = new IndexDisplay(c, ui->tabWidget);
  ui->tabWidget->addTab(indexDisplay, "Indexation");
  ui->tabWidget->addTab(new FitDisplay(c, ui->tabWidget), "Refinement");

  // creat MarkerModel
  markerModel = new LiveMarkerModel(c, this);

  ui->markerDisplay->setModel(markerModel);
  ui->markerDisplay->sortByColumn(markerModel->columnCount()-1, Qt::AscendingOrder);

  connect(markerModel, SIGNAL(doHighlightMarker(int)), this, SLOT(highlightMarkerNr(int)));
  connect(indexDisplay, SIGNAL(maxSearchIndexChanged(int)), markerModel, SLOT(maxSearchIndexChanged(int)));
  markerModel->maxSearchIndexChanged(indexDisplay->maxSearchIndex());

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
  //QSortFilterProxyModel* sorter = dynamic_cast<QSortFilterProxyModel*>(ui->markerDisplay->model());
  //QItemSelection selection = sorter->mapSelectionToSource(ui->markerDisplay->selectionModel()->selection());
  QItemSelection selection = ui->markerDisplay->selectionModel()->selection();
  for (int i=0; i<markerModel->rowCount(); i++) {
    markerModel->highlightMarker(i, selection.contains(markerModel->index(i, 0)));
  }
}

void HKLTool::highlightMarkerNr(int n) {
  //QSortFilterProxyModel* sorter = dynamic_cast<QSortFilterProxyModel*>(ui->markerDisplay->model());
  //ui->markerDisplay->setCurrentIndex(sorter->mapFromSource(markerModel->index(n, 0)));
  ui->markerDisplay->setCurrentIndex(markerModel->index(n, 0));
}

void HKLTool::deleteActiveMarker() {
  //QSortFilterProxyModel* sorter = dynamic_cast<QSortFilterProxyModel*>(ui->markerDisplay->model());
  //QModelIndex idx = sorter->mapToSource(ui->markerDisplay->selectionModel()->currentIndex());
  QModelIndex idx = ui->markerDisplay->selectionModel()->currentIndex();
  if (idx.isValid())
    markerModel->deleteMarker(idx.row());
}

