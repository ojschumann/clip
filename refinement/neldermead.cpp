#include "neldermead.h"

#include <QtConcurrentRun>
#include <QMetaType>
#include <iostream>

#include "core/crystal.h"
#include "core/projector.h"
#include "core/projectorfactory.h"
#include "tools/abstractmarkeritem.h"

using namespace std;

NelderMead::NelderMead(Crystal* c, QObject *parent) :
    QObject(parent),
    liveCrystal(c),
    shouldStop(false)
{
}

// Call only in thread.
//Signal connections need to be between objects in the worker thread
void NelderMead::init() {
  Crystal* fitCrystal = new Crystal();
  *fitCrystal = *liveCrystal;
  fitCrystal->enableUpdate(false);
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


void NelderMead::start() {
  threadWatcher.setFuture(QtConcurrent::run(this, &NelderMead::runWrapper));
}

void NelderMead::runWrapper() {
  // Funny stuff... runner now lives in this thread.
  // Thus it could receive signals from stuff created in initSimplex()

  NelderMead* runner = new NelderMead(liveCrystal);
  connect(runner, SIGNAL(newBestVertex(double, QList<double>)), this, SLOT(receiveSolution(double, QList<double>)), Qt::QueuedConnection);
  connect(this, SIGNAL(stopSignal()), runner, SLOT(receiveStop()), Qt::QueuedConnection);
  runner->init();
  runner->run();
  delete runner;
}

void NelderMead::stop() {
  emit stopSignal();
  threadWatcher.waitForFinished();
}

void NelderMead::receiveStop() {
  shouldStop = true;
}


double NelderMead::score() {
  foreach (FitParameter* p, parameters) p->setValue();
  double score=0;
  foreach (MarkerInfo m, markers) score += m.score(spotTransferMatrix, zoneTransferMatrix);
  return score;
}

double NelderMead::score(Vertex &v) {
  for (int n=0; n<v.size(); n++)
    parameters.at(n)->prepareValue(v.at(n));
  v.score = score();
  return v.score;
}


void NelderMead::run() {

  const int N = parameters.size();
  double alpha = 1.0;
  double beta = 0.5;
  double gamma = 2.0;

  // Downhill-Simplex-Verfahren from Nelder and Mead

  // Init the Simplex
  QList<Vertex> simplex;

  // Set actual Values of parameters as first simplex vertex
  Vertex v(N);
  for (int n=0; n<N; n++)
    v.coordinates[n]= parameters.at(n)->value();
  simplex << v;

  // Add N more Vertices, with exactely one parameter changed
  for (int n=0; n<N; n++) {
    Vertex t(v);
    t.coordinates[n] += 10.0*parameters.at(n)->epsilon();
    simplex << t;
  }

  // Score all Elements in the Simplex
  for (int i=0; i<simplex.size(); i++)
    score(simplex[i]);

  // And sort the Simplex...
  qSort(simplex);

  int loop = 0;
  forever {
    loop++;
    if (shouldStop) return;

    double bestLoopScore = simplex.first().score;

    // Take Worst Vertex
    Vertex W = simplex.takeLast();

    // Calculate center without worst element
    Vertex CoG;
    for (int n=0; n<simplex.size(); n++)
      CoG += simplex[n];
    CoG *= 1.0/simplex.size();


    // Reflect worst element about center and score
    Vertex R = CoG + (CoG - W)*alpha;
    score(R);

    if (simplex.first()<R && R<simplex.last()) {
      // Score is better than second-worst but not better than best
      // ToDo: insert at right place (bisect)
      simplex << R;
      qSort(simplex);
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
      // Move worst to the center
      Vertex C = W + (CoG - W)*beta;
      score(C);
      if (C<simplex.last()) {
        // if good, then take
        simplex << C;
        qSort(simplex);
      } else {
        // Shrink Simplex aroung best element
        simplex << W;
        for (int i=1; i<simplex.size(); i++) {
          simplex[i] = simplex.first() + (simplex[i] - simplex.first())*beta;
          score(simplex[i]);
        }
        qSort(simplex);
      }
    }

    if (simplex.first().score < bestLoopScore) {
      // We have a better Solution. Publish it
      emit newBestVertex(simplex.first().score, simplex.first().coordinates.toList());
    }

    QList<double> parameterDelta;
    bool allParameterBelowEpsilon=true;
    for (int m=0; m<N; m++) {
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
      parameterDelta << maxP-minP;
      if ((maxP-minP) > 1e-6*parameters.at(m)->epsilon()) allParameterBelowEpsilon=false;
    }
    if (allParameterBelowEpsilon) {
      cout << "All Parameters below Delta..." << endl;
      return;
    }
  }
}

void NelderMead::receiveSolution(double score, QList<double> solution) {
  cout << "Solution: " << score << endl;
}

void NelderMead::updateTransformationMatrices() {
  Crystal* fitCrystal = dynamic_cast<Crystal*>(copiedFitObjects.first());
  spotTransferMatrix = fitCrystal->getRealOrientationMatrix().transposed() * fitCrystal->getRotationMatrix().transposed();
  zoneTransferMatrix = fitCrystal->getReziprocalOrientationMatrix().transposed() * fitCrystal->getRotationMatrix().transposed();
}


NelderMead::MarkerInfo::MarkerInfo(AbstractMarkerItem *item):
    marker(item),
    index(item->getIntegerIndex().toType<double>()),
    index_sq(index.norm_sq())
{
}

double NelderMead::MarkerInfo::score(const Mat3D& spotTransfer, const Mat3D& zoneTransfer) const {
  Vec3D n = marker->getMarkerNormal();
  if (marker->getType()==AbstractMarkerItem::SpotMarker) {
    n = spotTransfer * n;
  } else if (marker->getType()==AbstractMarkerItem::ZoneMarker) {
    n = zoneTransfer * n;
  }
  n.normalize();
  double x = index*n;
  // TODO: Remove sqrt, not nessesary for fitting
  return sqrt(index_sq - x*x);
}

NelderMead::Vertex::Vertex() {
  score = -1;
}

NelderMead::Vertex::Vertex(int N) {
  score=-1;
  coordinates.resize(N);
  for (int n=0; n<N; n++) coordinates[n]=0.0;
}

NelderMead::Vertex::Vertex(const Vertex& o) {
  score = o.score;
  coordinates = o.coordinates;
}

NelderMead::Vertex& NelderMead::Vertex::operator=(const Vertex& o) {
  score = o.score;
  coordinates = o.coordinates;
  return *this;
}

bool NelderMead::Vertex::operator<(const Vertex& o) const {
  return score<o.score;
}

NelderMead::Vertex& NelderMead::Vertex::operator+=(const Vertex& o) {
  if (coordinates.empty()) coordinates.resize(o.coordinates.size());
  score = -1;
  for (int n=0; n<coordinates.size(); n++) coordinates[n] += o.coordinates.at(n);
  return *this;
}

NelderMead::Vertex& NelderMead::Vertex::operator-=(const Vertex& o) {
  if (coordinates.empty()) coordinates.resize(o.coordinates.size());
  score = -1;
  for (int n=0; n<coordinates.size(); n++) coordinates[n] -= o.coordinates.at(n);
  return *this;
}

NelderMead::Vertex& NelderMead::Vertex::operator*=(double scale) {
  score = -1;
  for (int n=0; n<coordinates.size(); n++) coordinates[n] *= scale;
  return *this;
}

NelderMead::Vertex NelderMead::Vertex::operator+(const Vertex& o) {
  if (coordinates.empty()) coordinates.resize(o.coordinates.size());
  Vertex tmp(*this);
  tmp += o;
  return tmp;
}

NelderMead::Vertex NelderMead::Vertex::operator-(const Vertex& o) {
  if (coordinates.empty()) coordinates.resize(o.coordinates.size());
  Vertex tmp(*this);
  tmp -= o;
  return tmp;
}

NelderMead::Vertex NelderMead::Vertex::operator*(double scale) const {
  Vertex tmp(*this);
  tmp *= scale;
  return tmp;
}

double NelderMead::Vertex::at(int n) const {
  return coordinates.at(n);
}

int NelderMead::Vertex::size() const {
  return coordinates.size();
}


int QListDoubleRegistered = qRegisterMetaType< QList<double> >();
