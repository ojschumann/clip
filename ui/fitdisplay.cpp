#include "fitdisplay.h"
#include "ui_fitdisplay.h"

#include <iostream>
#include <cmath>

#include "core/crystal.h"
#include "core/projector.h"
#include "core/projectorfactory.h"
#include "tools/abstractmarkeritem.h"
#include "tools/zipiterator.h"
#include "tools/spotitem.h"
#include "tools/zoneitem.h"
#include "tools/vec3D.h"

using namespace std;

FitDisplay::FitDisplay(Crystal* c, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::FitDisplay),
    crystal(c)
{
  ui->setupUi(this);

  fitCrystal = new Crystal();
  *fitCrystal = *crystal;

  connect(fitCrystal, SIGNAL(cellChanged()), this, SLOT(updateTransferMatrix()));
  connect(fitCrystal, SIGNAL(orientationChanged()), this, SLOT(updateTransferMatrix()));
  updateTransferMatrix();

  fitParameters += fitCrystal->allParameters();
  baseParameters += crystal->allParameters();

  foreach (Projector* p, crystal->getConnectedProjectors()) {
    if ((p->zoneMarkers().size()>0) || (p->spotMarkers().size()>0)) {
      Projector* fitP = ProjectorFactory::getInstance().getProjector(p->projectorName());
      *fitP = *p;
      fitParameters += fitP->allParameters();
      baseParameters += p->allParameters();
      QPair<ZoneItem*, ZoneItem*> zi;
      foreach(zi, Zip(fitP->zoneMarkers(), p->zoneMarkers())) {
        FitMarker m;
        m.marker = zi.first;
        m.index = zi.second->getIntegerIndex().toType<double>();
        m.index_sq = m.index.norm_sq();
        marker << m;
      }
      QPair<SpotItem*, SpotItem*> si;
      foreach(si, Zip(fitP->spotMarkers(), p->spotMarkers())) {
        FitMarker m;
        m.marker = si.first;
        m.index = si.second->getIntegerIndex().toType<double>();
        m.index_sq = m.index.norm_sq();
        marker << m;
      }
    }
  }
}

FitDisplay::~FitDisplay()
{
  delete ui;
}


void FitDisplay::updateTransferMatrix() {
  spotTransfer = fitCrystal->getRealOrientationMatrix().transposed() * fitCrystal->getRotationMatrix().transposed();
  zoneTransfer = fitCrystal->getReziprocalOrientationMatrix().transposed() * fitCrystal->getRotationMatrix().transposed();
}

double FitDisplay::score() {
  foreach (FitParameter* p, fitParameters) p->setValue();
  double score=0;
  foreach (FitMarker m, marker) {
    Vec3D n = m.marker->getMarkerNormal();
    if (m.marker->getType()==AbstractMarkerItem::SpotMarker) {
      n = spotTransfer * n;
    } else if (m.marker->getType()==AbstractMarkerItem::ZoneMarker) {
      n = zoneTransfer * n;
    }
    n.normalize();
    double x = m.index*n;
    // TODO: Remove sqrt, not nessesary for fitting
    score += sqrt(m.index_sq - x*x);
  }
  return score;
}

void FitDisplay::on_doFit_clicked()
{
  int N = fitParameters.size();
  double alpha = 1.0;
  double beta = 0.5;
  double gamma = 2.0;

  // Versuch des Downhill-Simplex-Verfahrens.....

  Vertex<N, this> v;
  for (int n=0; n<N; n++)
    v.coordinates[n]= fitParameters.at(n)->value();

  QList< Vertex<N, this> > simplex;

  simplex << v;

  for (int n=0; n<N; n++) {
    Vertex<N, this> t(v);
    t.coordinates[n] += fitParameters.at(n)->epsilon();
    simplex << t;
  }

  for (int i=0; i<simplex.size(); i++) {
    for (int n=0; n<N; n++) {
      fitParameters.at(n)->prepareValue(simplex[i].coordinates[n]);
    }
    simplex[i].score = score();
  }

  qSort(simplex);
  for (int i=0; i<N; i++) {
    baseParameters.at(i)->prepareValue(simplex.last().coordinates.at(i));
  }
  foreach (FitParameter* p, baseParameters) p->setValue();
  return;
  for (int loop=0; loop<30; loop++) {


    Vertex<N, this> M();
    for (int n=0; n<simplex.size()-1; n++)
      M += simplex[n];
    M *= 2.0/(simplex.size()-1);

    Vertex<N, this> L();
    L = M - simplex.last()*alpha;

    for (int n=0; n<N; n++) {
      fitParameters.at(n)->prepareValue(L.coordinates[n]);
    }
    L.score = score();
    if (L<simplex.first()) {
      Vertex<N, this> X();
      X = M - simplex.last()*gamma;
      for (int n=0; n<N; n++) {
        fitParameters.at(n)->prepareValue(X.coordinates[n]);
      }
      X.score = score();

      simplex.removeLast();
      if (X<L) {
        simplex.prepend(X);
      } else {
        simplex.prepend(L);
      }
    }


  }


}



template<int N, FitDisplay* fit> Vertex<N, fit>::Vertex() {
  score=-1;
  coordinates.resize(N);
  for (int n=0; n<N; n++) coordinates[n]=0.0;
}

template<int N, FitDisplay* fit> Vertex<N,fit>& Vertex<N, fit>::operator=(const Vertex<N, fit>& o) {
  score = o.score;
  coordinates = o.coordinates;
  return *this;
}

template<int N, FitDisplay* fit> Vertex<N, fit>::Vertex(const Vertex<N, fit>& o) {
  score = o.score;
  coordinates = o.coordinates;
}


template<int N, FitDisplay* fit> bool Vertex<N, fit>::operator<(const Vertex<N, fit>& o) const {
  return score<o.score;
}

template<int N, FitDisplay* fit> Vertex<N, fit>& Vertex<N, fit>::operator+=(const Vertex<N, fit>& o) {
  score = -1;
  for (int n=0; n<coordinates.size(); n++) coordinates[n] += o.coordinates.at(n);
  return *this;
}

template<int N, FitDisplay* fit> Vertex<N, fit>& Vertex<N, fit>::operator-=(const Vertex<N, fit>& o) {
  score = -1;
  for (int n=0; n<coordinates.size(); n++) coordinates[n] -= o.coordinates.at(n);
  return *this;
}

template<int N, FitDisplay* fit> Vertex<N, fit>& Vertex<N, fit>::operator*=(double scale) {
  score = -1;
  for (int n=0; n<coordinates.size(); n++) coordinates[n] *= scale;
  return *this;
}

template<int N, FitDisplay* fit> Vertex<N, fit> Vertex<N, fit>::operator+(const Vertex<N, fit>& o) const {
  Vertex<N, fit> tmp(*this);
  tmp += o;
  return tmp;
}

template<int N, FitDisplay* fit> Vertex<N, fit> Vertex<N, fit>::operator-(const Vertex<N, fit>& o) const {
  Vertex<N, fit> tmp(*this);
  tmp -= o;
  return tmp;
}

template<int N, FitDisplay* fit> Vertex<N, fit> Vertex<N, fit>::operator*(double scale) const {
  Vertex<N, fit> tmp(*this);
  tmp *= scale;
  return tmp;
}
