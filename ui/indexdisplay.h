#ifndef INDEXDISPLAY_H
#define INDEXDISPLAY_H

#include <QWidget>

#include "indexing/solutionmodel.h"
#include "indexing/solutionscorer.h"

namespace Ui {
    class Indexing;
}

class Crystal;

class IndexDisplay : public QWidget
{
    Q_OBJECT

public:
    explicit IndexDisplay(Crystal* _c, QWidget *parent = 0);
    virtual ~IndexDisplay();
signals:
    void stopIndexer();
private:
    Ui::Indexing *ui;
    Crystal* crystal;
    SolutionModel solutions;
    SolutionScorer solutionScorer;

    bool indexRunning;

private slots:
    void on_startButton_clicked();
    void updateSolutionDisplay(QModelIndex, QModelIndex);
    void indexerDestroyed();
    void showMajorIndex(int);
    void showNumberOfSolutions(int);
    void setProgress(int);
};

#endif // INDEXDISPLAY_H
