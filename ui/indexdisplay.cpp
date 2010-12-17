#include "indexdisplay.h"
#include "ui_indexdisplay.h"

#include <iostream>
#include <QThreadPool>

#include "core/crystal.h"
#include "core/projector.h"
#include "indexing/indexer.h"
using namespace std;

IndexDisplay::IndexDisplay(Crystal* _c, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Indexing),
    crystal(_c),
    solutions(),
    marker(this)
{
    ui->setupUi(this);

    ui->SolutionSelector->setModel(&solutions);
    ui->SolutionDisplay->setModel(&marker);


    ui->SolutionSelector ->verticalHeader()->setDefaultSectionSize(fontMetrics().lineSpacing());
    ui->SolutionDisplay->verticalHeader()->setDefaultSectionSize(fontMetrics().lineSpacing());

    ui->SolutionSelector->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    ui->SolutionDisplay->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    for (int n=6; n<9; n++)
      ui->SolutionDisplay->horizontalHeader()->setResizeMode(n, QHeaderView::Stretch);


    connect(ui->SolutionSelector->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)), this, SLOT(updateSolutionDisplay(QModelIndex,QModelIndex)));
    connect(&solutions, SIGNAL(solutionNumberChanged(int)), this, SLOT(showNumberOfSolutions(int)));

    indexRunning = false;

}

IndexDisplay::~IndexDisplay()
{
  emit stopIndexer();
  delete ui;
}

void IndexDisplay::updateSolutionDisplay(QModelIndex index, QModelIndex prev) {
  int n=index.row();
  if (n>=0) {
    Solution s = solutions.getSolution(n);
    marker.setSolution(s);
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

    QList<Vec3D> spotMarkerNormals;
    QList<Vec3D> zoneMarkerNormals;
    foreach (Projector* p, crystal->getConnectedProjectors()) {
      spotMarkerNormals += p->getSpotMarkerNormals();
      zoneMarkerNormals += p->getZoneMarkerNormals();
    }

    Indexer* indexer = new Indexer(spotMarkerNormals,
                                   zoneMarkerNormals,
                                   crystal->getRealOrientationMatrix(),
                                   crystal->getReziprocalOrientationMatrix(),
                                   M_PI/180.0*ui->AngDev->value(),
                                   0.01*ui->IntDev->value(),
                                   ui->MaxIdx->value(),
                                   crystal->getSpacegroup()->getLauegroup());
    connect(indexer, SIGNAL(publishSolution(Solution)), &solutions, SLOT(addSolution(Solution)));
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
  ui->maxIndex->setText("");
  ui->progress->setEnabled(false);
  ui->progress->setValue(0);

}

void IndexDisplay::showMajorIndex(int n) {
  ui->maxIndex->setText(QString::number(n));
  ui->progress->setMaximum((n+2)*(n+1)/2-1);
}

void IndexDisplay::setProgress(int value) {
  ui->progress->setValue(value);
}

void IndexDisplay::showNumberOfSolutions(int n) {
  ui->solutionsCount->setText(QString::number(n));
}
