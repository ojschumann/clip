#ifndef NELDERMEAD_WORKER_H
#define NELDERMEAD_WORKER_H

#include <QObject>
#include "refinement/neldermead.h"

class NMWorker: public QObject {
  Q_OBJECT
public:
  NMWorker(Crystal* c, QObject* parent=0);
  virtual ~NMWorker();

  void doOneIteration();
  QList<double> parameterDelta();
  QList<double> parameterRelativeDelta();
  QList<double> calcDeviation();
  QString parameterName(int n);
  QList<double> bestSolution();
  double bestScore();
  double worstScore();

  void restart();
protected slots:
  void updateTransformationMatrices();
protected:
  class Vertex {
  public:
    Vertex();
    Vertex(int N);
    Vertex(const Vertex& o);
    double score;
    QVector<double> coordinates;

    bool operator<(const Vertex& o) const;
    Vertex& operator=(const Vertex& o);
    Vertex& operator+=(const Vertex& o);
    Vertex& operator-=(const Vertex& o);
    Vertex& operator*=(double scale);
    Vertex operator+(const Vertex& o);
    Vertex operator-(const Vertex& o);
    Vertex operator*(double scale) const;

    double at(int n) const;
    int size() const;
  };
  class MarkerInfo {
  public:
    MarkerInfo(AbstractMarkerItem* item);
    double score(const Mat3D& spotTransfer, const Mat3D& zoneTransfer) const;
  //protected:
    AbstractMarkerItem* marker;
    Vec3D index;
    double index_sq;
  };

  void initParameter();
  void initSimplex();

  double score();
  double score(Vertex&);
  double score_c(const Vertex&);

  // Crystal, that is used in the UI
  Crystal* liveCrystal;

  // The thing to optimize
  QList<Vertex> simplex;

  // Matrices to transfer normals to indices
  Mat3D spotTransferMatrix;
  Mat3D zoneTransferMatrix;

  // local copies of crystal and projectors
  QList<FitObject*> copiedFitObjects;
  QList<FitParameter*> parameters;
  QList<MarkerInfo> markers;

};



#endif // NELDERMEAD_WORKER_H
