#include "indexdisplay.h"
#include "ui_indexdisplay.h"

#include <iostream>
#include <iomanip>
#include <QThreadPool>
#include <QSortFilterProxyModel>
#include <QShortcut>

#include "core/crystal.h"
#include "indexing/indexer.h"
#include "indexing/livemarkermodel.h"
#include "tools/zipiterator.h"


using namespace std;

IndexDisplay::IndexDisplay(Crystal* _c, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Indexing),
    crystal(_c),
    solutions()
{
  ui->setupUi(this);

  ui->SolutionSelector->setModel(&solutions);
  ui->SolutionSelector->sortByColumn(0, Qt::AscendingOrder);

  ui->SolutionSelector ->verticalHeader()->setDefaultSectionSize(fontMetrics().lineSpacing());

  ui->SolutionSelector->horizontalHeader()->setResizeMode(QHeaderView::Stretch);

  connect(ui->SolutionSelector->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), this, SLOT(updateSolutionDisplay(QModelIndex,QModelIndex)));
  connect(&solutions, SIGNAL(solutionNumberChanged(int)), this, SLOT(showNumberOfSolutions(int)));
  connect(ui->maxIndex, SIGNAL(valueChanged(int)), this, SIGNAL(maxSearchIndexChanged(int)));
  indexRunning = false;

}

IndexDisplay::~IndexDisplay()
{
  emit stopIndexer();
  delete ui;
}

int IndexDisplay::maxSearchIndex() {
  return ui->maxIndex->value();
}

void IndexDisplay::updateSolutionDisplay(QModelIndex index, QModelIndex prev) {
  int n=index.row();
  if (n>=0) {
    Solution s = solutions.getSolution(n);
    crystal->setRotation(s.bestRotation);
  }
}

void IndexDisplay::on_startButton_clicked()
{
  if (indexRunning) {
    emit stopIndexer();
  } else {
    ui->startButton->setText("Stop");
    ui->progress->setEnabled(true);
    ui->progress->setValue(0);

    Indexer* indexer = new Indexer(crystal->getMarkers(),
                                   crystal->getRealOrientationMatrix(),
                                   crystal->getReziprocalOrientationMatrix(),
                                   M_PI/180.0*ui->AngDev->value(),
                                   0.01*ui->IntDev->value(),
                                   ui->maxIndex->value(),
                                   crystal->getSpacegroup()->getLauegroup());
    connect(indexer, SIGNAL(publishSolution(Solution)), &solutions, SLOT(addSolution(Solution)));
    connect(indexer, SIGNAL(publishMultiSolutions(QList<Solution>)), &solutions, SLOT(addSolutions(QList<Solution>)));
    connect(indexer, SIGNAL(destroyed()), this, SLOT(indexerDestroyed()));
    connect(indexer, SIGNAL(nextMajorIndex(int)), this, SLOT(showMajorIndex(int)));
    connect(indexer, SIGNAL(progressInfo(int)), this, SLOT(setProgress(int)));
    connect(this, SIGNAL(stopIndexer()), indexer, SLOT(stop()));
    solutions.clear();
    indexRunning = true;

    QThreadPool::globalInstance()->start(indexer);
  }
}

void IndexDisplay::indexerDestroyed() {
  indexRunning=false;
  ui->startButton->setText("Start");
  ui->maxIndexDisplay->setText("");
  ui->progress->setEnabled(false);
  ui->progress->setValue(0);

}

void IndexDisplay::showMajorIndex(int n) {
  ui->maxIndexDisplay->setText(QString::number(n));
  ui->progress->setMaximum((n+2)*(n+1)/2-1);
}

void IndexDisplay::setProgress(int value) {
  ui->progress->setValue(value);
}

void IndexDisplay::showNumberOfSolutions(int n) {
  ui->solutionsCount->setText(QString::number(n));
}


