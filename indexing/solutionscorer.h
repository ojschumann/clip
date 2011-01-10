#ifndef SOLUTIONSCORER_H
#define SOLUTIONSCORER_H

#include <QObject>

#include "indexing/solution.h"

class Crystal;

class SolutionScorer : public QObject
{
  Q_OBJECT
public:
  explicit SolutionScorer(QObject *parent = 0);
  virtual ~SolutionScorer();

  void copyCrystal(Crystal* c);
signals:
  void solutionScored(Solution);
public slots:
  void scoreSolution(Solution);
protected:
  void deleteCrystal();

private:
  Crystal* copiedCrystal;

};

#endif // SOLUTIONSCORER_H
