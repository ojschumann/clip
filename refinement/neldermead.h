#ifndef NELDERMEAD_H
#define NELDERMEAD_H

#include <QObject>
#include <QFutureWatcher>

#include "tools/vec3D.h"
#include "tools/mat3D.h"

class FitObject;
class FitParameter;
class Crystal;
class AbstractMarkerItem;

class NelderMead : public QObject
{
  Q_OBJECT
public:
  explicit NelderMead(Crystal* c, QObject *parent = 0);
public slots:
  void start();
  void stop();
signals:
  void newBestVertex(QList<double>);
  void stopSignal();
protected slots:
  void receiveSolution(QList<double>);
  void updateTransformationMatrices();
  void receiveStop();
protected:
  void init();
  void runWrapper();
  void run();

  double score();
  double score(Vertex&);

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
    MarkerInfo(AbstractMarkerItem* item, const Vec3D& index);
    double score(const Mat3D& spotTransfer, const Mat3D& zoneTransfer) const;
  protected:
    AbstractMarkerItem* marker;
    Vec3D index;
    double index_sq;
  };


  // The thing to optimize
  QList<Vertex> simplex;

  // Matrices to transfer normals to indices
  Mat3D spotTransferMatrix;
  Mat3D zoneTransferMatrix;

  // local copies of crystal and projectors
  QList<FitObject*> copiedFitObjects;
  QList<FitParameter*> parameters;
  QList<MarkerInfo*> markers;

  // Crystal, that is used in the UI
  Crystal* liveCrystal;

  bool shouldStop;

  QFutureWatcher<void> threadWatcher;

};

#endif // NELDERMEAD_H
