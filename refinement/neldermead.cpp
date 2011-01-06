#include "neldermead.h"

NelderMead::NelderMead(QObject *parent) :
    QObject(parent)
{
}


/*
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
  fitCrystal->enableUpdate(false);

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

double FitDisplay::score(Vertex &v) {
  for (int n=0; n<v.size(); n++)
    fitParameters.at(n)->prepareValue(v.at(n));
  v.score = score();
  return v.score;
}

void FitDisplay::on_doFit_clicked()
{
  int N = fitParameters.size();
  double alpha = 1.0;
  double beta = 0.5;
  double gamma = 2.0;

  // Versuch des Downhill-Simplex-Verfahrens.....

  Vertex v(N);
  for (int n=0; n<N; n++)
    v.coordinates[n]= fitParameters.at(n)->value();

  QList<Vertex> simplex;

  simplex << v;

  for (int n=0; n<N; n++) {
    Vertex t(v);
    t.coordinates[n] += 10.0*fitParameters.at(n)->epsilon();
    simplex << t;
  }

  for (int i=0; i<simplex.size(); i++) {
    score(simplex[i]);
  }

  qSort(simplex);

  for (int loop=0; loop<100; loop++) {
    cout << simplex.first().score << endl;
    // Take Worst Vertex

    Vertex W = simplex.takeLast();
    // Calc center without worst element
    Vertex CoG;
    for (int n=0; n<simplex.size(); n++)
      CoG += simplex[n];
    CoG *= 1.0/simplex.size();


    // Reflect worst element about center
    Vertex R = CoG + (CoG - W)*alpha;
    score(R);
    // Score is better than best...
    if (simplex.first()<R && R<simplex.last()) {
      // ToDo: insert at right place
      simplex << R;
      qSort(simplex);
    } else if (R<simplex.first()) {
      Vertex E = CoG + (CoG - W)*gamma;
      score(E);
      if (E<R) {
        simplex.prepend(E);
      } else {
        simplex.prepend(R);
      }
    } else {
      Vertex C = W + (CoG - W)*beta;
      score(C);
      if (C<simplex.last()) {
        simplex << C;
        qSort(simplex);
      } else {
        simplex << W;
        for (int i=1; i<simplex.size(); i++) {
          simplex[i] = simplex.first() + (simplex[i] - simplex.first())*beta;
          score(simplex[i]);
        }
        qSort(simplex);
      }
    }


  }
  for (int n=0; n<N; n++) baseParameters.at(n)->prepareValue(simplex.first().at(n));
  for (int n=0; n<N; n++) baseParameters.at(n)->setValue();

}


FitDisplay::Vertex::Vertex() {
  score = -1;
}

FitDisplay::Vertex::Vertex(int N) {
  score=-1;
  coordinates.resize(N);
  for (int n=0; n<N; n++) coordinates[n]=0.0;
}

FitDisplay::Vertex::Vertex(const Vertex& o) {
  score = o.score;
  coordinates = o.coordinates;
}

FitDisplay::Vertex& FitDisplay::Vertex::operator=(const Vertex& o) {
  score = o.score;
  coordinates = o.coordinates;
  return *this;
}

bool FitDisplay::Vertex::operator<(const Vertex& o) const {
  return score<o.score;
}

FitDisplay::Vertex& FitDisplay::Vertex::operator+=(const Vertex& o) {
  if (coordinates.empty()) coordinates.resize(o.coordinates.size());
  score = -1;
  for (int n=0; n<coordinates.size(); n++) coordinates[n] += o.coordinates.at(n);
  return *this;
}

FitDisplay::Vertex& FitDisplay::Vertex::operator-=(const Vertex& o) {
  if (coordinates.empty()) coordinates.resize(o.coordinates.size());
  score = -1;
  for (int n=0; n<coordinates.size(); n++) coordinates[n] -= o.coordinates.at(n);
  return *this;
}

FitDisplay::Vertex& FitDisplay::Vertex::operator*=(double scale) {
  score = -1;
  for (int n=0; n<coordinates.size(); n++) coordinates[n] *= scale;
  return *this;
}

FitDisplay::Vertex FitDisplay::Vertex::operator+(const Vertex& o) {
  if (coordinates.empty()) coordinates.resize(o.coordinates.size());
  Vertex tmp(*this);
  tmp += o;
  return tmp;
}

FitDisplay::Vertex FitDisplay::Vertex::operator-(const Vertex& o) {
  if (coordinates.empty()) coordinates.resize(o.coordinates.size());
  Vertex tmp(*this);
  tmp -= o;
  return tmp;
}

FitDisplay::Vertex FitDisplay::Vertex::operator*(double scale) const {
  Vertex tmp(*this);
  tmp *= scale;
  return tmp;
}

double FitDisplay::Vertex::at(int n) const {
  return coordinates.at(n);
}

int FitDisplay::Vertex::size() const {
  return coordinates.size();
}
*/
