#include "neldermead_worker.h"

#include <cstdlib>

#include "core/crystal.h"
#include "core/projector.h"
#include "core/projectorfactory.h"
#include "tools/abstractmarkeritem.h"
#include "refinement/fitparameter.h"
#include "refinement/fitparametergroup.h"

NMWorker::NMWorker(Crystal* c, QObject* parent):
    QObject(parent),
    liveCrystal(c)
{
  initParameter();
  initSimplex();
}

NMWorker::~NMWorker() {
  foreach (FitObject* o, copiedFitObjects) delete o;
}

double NMWorker::score() {
  foreach (FitParameter* p, parameters) {
    p->setValue();
  }
  double score=0;
  foreach (MarkerInfo m, markers) score += m.score(spotTransferMatrix, zoneTransferMatrix);
  //foreach (MarkerInfo m, markers) score += m.marker->getIndexDeviationScore();
  //foreach (MarkerInfo m, markers) score += m.marker->getAngularDeviation();
  //foreach (MarkerInfo m, markers) score += m.marker->getDetectorPositionScore();
  return score;
}

double NMWorker::score(Vertex &v) {
  for (int n=0; n<v.size(); n++)
    parameters.at(n)->prepareValue(v.at(n));
  v.score = score();
  return v.score;
}

double NMWorker::bestScore() {
  return simplex.first().score;
}

QList<double> NMWorker::bestSolution() {
  return simplex.first().coordinates.toList();
}

QList<double> NMWorker::parameterDelta() {
  QList<double> delta;
  for (int m=0; m<parameters.size(); m++) {
    double minP=simplex.at(0).at(m);
    double maxP=minP;
    for (int n=1; n<simplex.size(); n++) {
      double p = simplex.at(n).at(m);
      if (p>maxP) {
        maxP=p;
      } else if (p<minP){
        minP=p;
      }
    }
    delta << maxP-minP;
  }
  return delta;
}

QList<double> NMWorker::parameterRelativeDelta() {
  QList<double> list = parameterDelta();
  for (int n=0; n<list.size(); n++) {
    list[n] /= parameters.at(n)->epsilon();
  }
  return list;
}

QString NMWorker::parameterName(int n) {
  if (n>=0 && n<parameters.size()) {
    return parameters.at(n)->name();
  }
  return QString();
}

void NMWorker::restart() {
  // This sets the Parameters to those of the best score
  score(simplex.first());
  // Clears the Simplex
  simplex.clear();
  //And reinits it
  initSimplex();
}

void NMWorker::initParameter() {
  Crystal* fitCrystal = new Crystal();
  *fitCrystal = *liveCrystal;
  fitCrystal->enableUpdate(false);
  fitCrystal->prepareForFit();
  liveCrystal->prepareForFit();
  copiedFitObjects << fitCrystal;

  connect(fitCrystal, SIGNAL(cellChanged()), this, SLOT(updateTransformationMatrices()));
  connect(fitCrystal, SIGNAL(orientationChanged()), this, SLOT(updateTransformationMatrices()));
  updateTransformationMatrices();

  parameters += fitCrystal->enabledParameters();

  foreach (Projector* liveP, liveCrystal->getConnectedProjectors()) {
    if (liveP->hasMarkers()) {
      Projector* fitP = ProjectorFactory::getInstance().getProjector(liveP->projectorName());
      *fitP = *liveP;
      fitP->connectToCrystal(fitCrystal);
      copiedFitObjects << fitP;
      parameters += fitP->enabledParameters();

      foreach (AbstractMarkerItem* m, fitP->getAllMarkers()) {
        markers << MarkerInfo(m);
      }
    }
  }
}

void NMWorker::initSimplex() {
  const int N = parameters.size();

  // Downhill-Simplex-Verfahren from Nelder and Mead

  // Set actual Values of parameters as first simplex vertex
  Vertex v(N);
  for (int n=0; n<N; n++)
    v.coordinates[n]= parameters.at(n)->value();
  simplex << v;

  // Add N more Vertices, with exactely one parameter changed
  for (int n=0; n<N; n++) {
    Vertex t(v);
    double factor = 5000.0*qrand()/RAND_MAX - 2500.0;
    t.coordinates[n] += factor*parameters.at(n)->epsilon();
    simplex << t;
  }

  // Score all Elements in the Simplex
  for (int i=0; i<simplex.size(); i++)
    score(simplex[i]);

  // And sort the Simplex...
  qSort(simplex);
}

void NMWorker::doOneIteration() {
  double alpha = 1.0;
  double beta = 0.5;
  double gamma = 2.0;

  // Take Worst Vertex
  Vertex W = simplex.takeLast();

  // Calculate center of Gravity (CoG) without the worst element
  Vertex CoG;
  for (int n=0; n<simplex.size(); n++)
    CoG += simplex[n];
  CoG *= 1.0/simplex.size();


  // Reflect worst element about center and score
  Vertex R = CoG + (CoG - W)*alpha;
  score(R);

  if (simplex.first()<R && R<simplex.last()) {
    // Score is better than second-worst but not better than best
    // Insert at right position
    simplex.insert(qLowerBound(simplex, R) - simplex.begin(), R);
    //simplex << R;
    //qSort(simplex);
  } else if (R<simplex.first()) {
    // Score is better than best, try to extend further
    Vertex E = CoG + (CoG - W)*gamma;
    score(E);
    // add best
    if (E<R) {
      simplex.prepend(E);
    } else {
      simplex.prepend(R);
    }
  } else {
    // Move worst element nearer to the center
    Vertex C = W + (CoG - W)*beta;
    score(C);
    if (C<simplex.last()) {
      // if good, then take
      simplex.insert(qLowerBound(simplex, C) - simplex.begin(), C);
    } else {
      // Shrink Simplex around best element
      simplex << W;
      for (int i=1; i<simplex.size(); i++) {
        simplex[i] = simplex.first() + (simplex[i] - simplex.first())*beta;
        score(simplex[i]);
      }
      qSort(simplex);
    }
  }
}


void NMWorker::updateTransformationMatrices() {
  Crystal* fitCrystal = dynamic_cast<Crystal*>(copiedFitObjects.first());
  spotTransferMatrix = fitCrystal->getRealOrientationMatrix().transposed() * fitCrystal->getRotationMatrix().transposed();
  zoneTransferMatrix = fitCrystal->getReziprocalOrientationMatrix().transposed() * fitCrystal->getRotationMatrix().transposed();
}



NMWorker::MarkerInfo::MarkerInfo(AbstractMarkerItem *item):
    marker(item),
    index(item->getIntegerIndex().toType<double>()),
    index_sq(index.norm_sq())
{
}

double NMWorker::MarkerInfo::score(const Mat3D& spotTransfer, const Mat3D& zoneTransfer) const {
  Vec3D n = marker->getMarkerNormal();
  if (marker->getType()==AbstractMarkerItem::SpotMarker) {
    n = spotTransfer * n;
  } else if (marker->getType()==AbstractMarkerItem::ZoneMarker) {
    n = zoneTransfer * n;
  }
  n.normalize();
  double x = index*n;
  return index_sq - x*x;
}

NMWorker::Vertex::Vertex() {
  score = -1;
}

NMWorker::Vertex::Vertex(int N) {
  score=-1;
  coordinates.resize(N);
  for (int n=0; n<N; n++) coordinates[n]=0.0;
}

NMWorker::Vertex::Vertex(const Vertex& o) {
  score = o.score;
  coordinates = o.coordinates;
}

NMWorker::Vertex& NMWorker::Vertex::operator=(const Vertex& o) {
  score = o.score;
  coordinates = o.coordinates;
  return *this;
}

bool NMWorker::Vertex::operator<(const Vertex& o) const {
  return score<o.score;
}

NMWorker::Vertex& NMWorker::Vertex::operator+=(const Vertex& o) {
  if (coordinates.empty()) coordinates.resize(o.coordinates.size());
  score = -1;
  for (int n=0; n<coordinates.size(); n++) coordinates[n] += o.coordinates.at(n);
  return *this;
}

NMWorker::Vertex& NMWorker::Vertex::operator-=(const Vertex& o) {
  if (coordinates.empty()) coordinates.resize(o.coordinates.size());
  score = -1;
  for (int n=0; n<coordinates.size(); n++) coordinates[n] -= o.coordinates.at(n);
  return *this;
}

NMWorker::Vertex& NMWorker::Vertex::operator*=(double scale) {
  score = -1;
  for (int n=0; n<coordinates.size(); n++) coordinates[n] *= scale;
  return *this;
}

NMWorker::Vertex NMWorker::Vertex::operator+(const Vertex& o) {
  if (coordinates.empty()) coordinates.resize(o.coordinates.size());
  Vertex tmp(*this);
  tmp += o;
  return tmp;
}

NMWorker::Vertex NMWorker::Vertex::operator-(const Vertex& o) {
  if (coordinates.empty()) coordinates.resize(o.coordinates.size());
  Vertex tmp(*this);
  tmp -= o;
  return tmp;
}

NMWorker::Vertex NMWorker::Vertex::operator*(double scale) const {
  Vertex tmp(*this);
  tmp *= scale;
  return tmp;
}

double NMWorker::Vertex::at(int n) const {
  return coordinates.at(n);
}

int NMWorker::Vertex::size() const {
  return coordinates.size();
}
