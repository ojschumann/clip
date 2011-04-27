/**********************************************************************
  Copyright (C) 2010 Olaf J. Schumann

  This file is part of the Cologne Laue Indexation Program.
  For more information, see <http://clip.berlios.de>

  Clip is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  Clip is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see http://www.gnu.org/licenses/
  or write to the Free Software Foundation, Inc., 51 Franklin Street,
  Fifth Floor, Boston, MA 02110-1301, USA.
 **********************************************************************/


#include <cmath>
#include <iostream>
#include <QTime>
#include <QMetaObject>
#include <QtConcurrentMap>
#include <QtConcurrentRun>
#include <QSettings>

#include "tools/tools.h"
#include "core/crystal.h"
#include "core/projector.h"
#include "tools/optimalrotation.h"
#include "core/reflection.h"
#include "core/spacegroup.h"
#include "tools/xmltools.h"
#include "tools/abstractmarkeritem.h"
#include "refinement/fitparameter.h"
using namespace std;





class IntIterator {
public:
  IntIterator(int _pos): pos(_pos) {}
  int pos;
  typedef std::random_access_iterator_tag iterator_category;
  typedef int value_type;
  typedef int difference_type;
  typedef void pointer;
  typedef void reference;

  bool operator !=(const IntIterator& o) {
    return pos != o.pos;
  }

  void operator ++() {
    pos++;
  }

  void operator+=(int n) {
    pos +=n;
  }

  int operator *() {
    return pos;
  }

  int operator-(IntIterator& o) {
    return pos-o.pos;
  }

};


Crystal::Crystal(QObject* parent):
    FitObject(parent),
    MReal(),
    MReziprocal(),
    MRot(),
    connectedProjectors(this),
    rotationAxis(1,0,0),
    spaceGroup(this),
    reflections(),
    cellGroup(this),
    orientationGroup(this)
{
  QSettings settings;
  settings.beginGroup("Crystal");
  spaceGroup.setGroupSymbol(settings.value("Spacegroup", "P1").toString());
  internalSetCell(settings.value("CellA", 4.0).toDouble(),
                  settings.value("CellB", 4.0).toDouble(),
                  settings.value("CellC", 4.0).toDouble(),
                  settings.value("CellAlpha", 90.0).toDouble(),
                  settings.value("CellBeta", 90.0).toDouble(),
                  settings.value("CellGamma", 90.0).toDouble());
  setEulerAngles(settings.value("EulerOmega", 00.0).toDouble(),
                 settings.value("EulerChi", 00.0).toDouble(),
                 settings.value("EulerPhi", 00.0).toDouble());

  settings.endGroup();

  Qmin=0.0;
  Qmax=1.0;
  predictionFactor = 1.0;
  connect(&connectedProjectors, SIGNAL(objectAdded()), this, SLOT(updateWavevectorsFromProjectors()));
  connect(&connectedProjectors, SIGNAL(objectRemoved()), this, SLOT(updateWavevectorsFromProjectors()));
  connect(&spaceGroup, SIGNAL(constrainsChanged()), this, SLOT(slotSetSGConstrains()));
  connect(&spaceGroup, SIGNAL(triclinHtoR()), this, SLOT(convertHtoR()));
  connect(&spaceGroup, SIGNAL(triclinRtoH()), this, SLOT(convertRtoH()));
  connect(&spaceGroup, SIGNAL(groupChanged()),this, SLOT(generateReflections()));
  connect(&reflectionFuture, SIGNAL(finished()), this, SLOT(reflectionGenerated()));
  axisType=LabSystem;
  enableUpdate();
  synchronUpdate();
  restartReflectionUpdate = false;
  generateReflections();

  addParameterGroup(&cellGroup);
  addParameterGroup(&orientationGroup);

  connect(this, SIGNAL(cellChanged()), &cellGroup, SLOT(groupDataChanged()));
  connect(this, SIGNAL(orientationChanged()), &orientationGroup, SLOT(groupDataChanged()));
}

Crystal& Crystal::operator=(const Crystal& c) {
  synchronUpdate();
  spaceGroup.setGroupSymbol(c.spaceGroup.groupSymbol());
  internalSetCell(c.a,c.b,c.c,c.alpha,c.beta,c.gamma);
  setWavevectors(c.Qmin, c.Qmax);
  predictionFactor = 1.0;

  setRotation(c.getRotationMatrix());
  setRotationAxis(c.getRotationAxis(), c.getRotationAxisType());
  synchronUpdate(c.updateIsSynchron);
  FitObject::operator=(c);
  return *this;
}

Crystal::~Crystal() {}

QString Crystal::FitObjectName() {
  return "Crystal";
}

void Crystal::prepareForFit() {
  orientationGroup.setBaseRotation(getRotationMatrix());
}

void Crystal::setCell(double _a, double _b, double _c, double _alpha, double _beta, double _gamma) {
  QList<double> c;
  c << _a << _b << _c << _alpha << _beta << _gamma;
  setCell(c);
}

void Crystal::setCell(QList<double> cell) {
  if (cell.size()!=6) return;
  QList<int> constrains = spaceGroup.getConstrains();
  for (int i=0; i<6; i++) {
    if (constrains[i]>0) cell[i]=constrains[i];
    if (constrains[i]<0) cell[i]=cell[-constrains[i]-1];
  }

  double _a     = 0.00001*floor(100000.0*cell[0]+0.5);
  double _b     = 0.00001*floor(100000.0*cell[1]+0.5);
  double _c     = 0.00001*floor(100000.0*cell[2]+0.5);
  double _alpha = 0.00001*floor(100000.0*cell[3]+0.5);
  double _beta  = 0.00001*floor(100000.0*cell[4]+0.5);
  double _gamma = 0.00001*floor(100000.0*cell[5]+0.5);

  // discard minor changes (may affect fit)
  if (fabs(_a-a)>1e-8 || fabs(_b-b)>1e-8 || fabs(_c-c)>1e-8 || fabs(_alpha-alpha)>1e-8 ||fabs(_beta-beta)>1e-8 || fabs(_gamma-gamma)>1e-8) {
    internalSetCell(_a, _b, _c, _alpha, _beta, _gamma);
  }
}

void Crystal::internalSetCell(double _a, double _b, double _c, double _alpha, double _beta, double _gamma) {
  a=_a;
  b=_b;
  c=_c;
  alpha=_alpha;
  beta=_beta;
  gamma=_gamma;

  // Cosini und Sini of angles between lattice vectors
  double Ca=cos(M_PI/180.0*alpha);
  double Cb=cos(M_PI/180.0*beta);
  double Cc=cos(M_PI/180.0*gamma);
  double Sc=sin(M_PI/180.0*gamma);

  // volume of the unit cell in real abnd reziprocal space
  //double Vreal=a*b*c*sqrt(1.0+2.0*Ca*Cb*Cc-Ca*Ca-Cb*Cb-Cc*Cc);
  //double Vrezi=1.0/Vreal;

  // in real space, a directs in x-direction, b is in the xy-plane and c completes the right handed set
  Vec3D a_real(a, 0, 0);
  Vec3D b_real(Cc*b, Sc*b, 0);
  Vec3D c_real(Cb, (Ca-Cb*Cc)/Sc, sqrt(1.0-Cb*Cb-(Ca-Cb*Cc)/Sc*(Ca-Cb*Cc)/Sc));
  c_real *= c;

  MReal=Mat3D(a_real, b_real, c_real);

  // reciprocal orientation matrix is inverse transposed of real one!
  MReziprocal = MReal.inverse();
  MReziprocal.transpose();
  // Extract reciprocal basis vecors
  astar=MReziprocal(0);
  bstar=MReziprocal(1);
  cstar=MReziprocal(2);
  generateReflections();
  emit cellChanged();
}

void Crystal::addRotation(const Vec3D& axis, double angle) {
  addRotation(Mat3D(axis, angle));
}

void Crystal::addRotation(const Mat3D& M) {
  // Multiply from left and orthogonalize to suppress rounding errors
  MRot.lmult(M);
  MRot.orthogonalize();
  updateRotation();
  emit orientationChanged();
}

void Crystal::setRotation(const Mat3D& M) {
  MRot = M;
  updateRotation();
  emit orientationChanged();
}

void Crystal::setWavevectors(double _Qmin, double _Qmax) {
  if ((_Qmin<_Qmax) and ((_Qmin!=Qmin) or (_Qmax!=Qmax))) {
    Qmin=_Qmin;
    Qmax=_Qmax;
    generateReflections();
  }
}

void Crystal::generateReflections() {
  if (not updateEnabled)
    return;
  if (updateIsSynchron) {
    reflections = doGeneration();
    emit reflectionsUpdate();
  } else if (reflectionFuture.isRunning()) {
    restartReflectionUpdate = true;
  } else {
    reflectionFuture.setFuture(QtConcurrent::run(this, &Crystal::doGeneration));
  }
}

void Crystal::reflectionGenerated() {
  reflections = reflectionFuture.result();
  if (restartReflectionUpdate) {
    restartReflectionUpdate = false;
    generateReflections();
  }
  emit reflectionsUpdate();
}

QVector<Reflection> Crystal::doGeneration() {
  Vec3D astar(this->astar);
  Vec3D bstar(this->bstar);
  Vec3D cstar(this->cstar);
  Mat3D MReziprocal(this->MReziprocal);
  double Qmax(this->Qmax);

  // Qmax =2*pi/lambda_min
  // n*lambda=2*d*sin(theta) => n_max=2*d/lambda = Qmax*d/pi
  int hMax = int(M_1_PI*a*Qmax);
  // predicted number of reflections
  double prediction = 4.0/3.0*M_1_PI*M_1_PI*Qmax*Qmax*Qmax*MReal.det();
  Crystal::UpdateRef updateRef(this);
  QVector<Reflection> refs;
  refs.reserve(int(1.1*predictionFactor*prediction));
  Spacegroup sg(spaceGroup);
  for (int h=-hMax; h<hMax; h++) {
    //|h*as+k*bs|^2=h^2*|as|^2+k^2*|bs|^2+2*h*k*as*bs==(2*Qmax)^2
    // k^2 +2*k*h*as*bs/|bs|^2 + (h^2*|as|^2-4*Qmax^2)/|bs|^2 == 0
    double ns = 1.0/bstar.norm_sq();
    double p = astar*bstar*ns*h;
    double q1 = astar.norm_sq()*ns*h*h;
    double q2 = M_1_PI*M_1_PI*ns*Qmax*Qmax;
    double s = p*p-q1+q2;
    int kMin = (s>0)?int(-p-sqrt(s)):0;
    int kMax = (s>0)?int(-p+sqrt(s)):0;

    for (int k=kMin; k<=kMax; k++) {

      int hk_ggt = ggt(h,k);

      Vec3D v = MReziprocal*Vec3D(h,k,0);
      ns = 1.0/cstar.norm_sq();
      p = v*cstar*ns;
      q1 = v.norm_sq()*ns;
      q2 = M_1_PI*M_1_PI*ns*Qmax*Qmax;
      s = p*p-q1+q2;
      int lMin = (s>0)?int(-p-sqrt(s)):0;
      int lMax = (s>0)?int(-p+sqrt(s)):0;

      for (int l=lMin; l<=lMax; l++) {
        // store only lowest order reflections
        if (ggt(hk_ggt, l)==1) {
          v=MReziprocal*Vec3D(h,k,l);
          double Q = 2.0*M_PI*v.norm();

          Reflection r;
          r.h=h;
          r.k=k;
          r.l=l;
          r.hklSqSum=h*h+k*k+l*l;
          r.Q=Q;
          r.d = 2.0*M_PI/Q;
          for (int i=1; i<=int(M_1_PI*Qmax*r.d+0.9); i++) {
            if (!sg.isExtinct(TVec3D<int>(i*h, i*k, i*l)))
              r.orders.push_back(i);
          }
          if (r.orders.size()>0) {
            r.normalLocal=v*r.d;
            updateRef(r);
            refs += r;
          }
        }
      }
    }
  }
  predictionFactor = 0.8 * predictionFactor + 0.2 * (refs.size()/prediction);
  return refs;
}


Reflection Crystal::makeReflection(const TVec3D<int> &_hkl) const {
  int _ggt = ggt(_hkl.x(), ggt(_hkl.y(), _hkl.z()));
  if (_ggt<=1) _ggt = 1;

  TVec3D<int> hkl = hkl / _ggt;

  Vec3D v=MReziprocal*hkl.toType<double>();
  double Q = 2.0*M_PI*v.norm();

  Reflection r;
  r.h=hkl.x();
  r.k=hkl.y();
  r.l=hkl.z();
  r.hklSqSum=hkl.norm_sq();
  r.Q=Q;
  r.d = 2.0*M_PI/Q;
  for (int i=1; i<=int(M_1_PI*Qmax*r.d+0.9); i++) {
    if (!spaceGroup.isExtinct(hkl*i))
      r.orders.push_back(i);
  }
  r.normalLocal=v*r.d;
  Crystal::UpdateRef updateRef(this);
  updateRef(r);
  return r;
}

Crystal::UpdateRef::UpdateRef(const Crystal *c) {
  MRot = c->MRot;
  Qmin = c->Qmin;
  Qmax = c->Qmax;
}

void Crystal::UpdateRef::operator()(Reflection &r) {
  r.normal=MRot*r.normalLocal;
  r.lowestDiffOrder=0;
  r.highestDiffOrder=0;

  // sin(theta) = v*e_x = v.x
  // x direction points toward source, z points upwards
  if (r.normal.x()>0.0) {
    //Q=2*pi/d/sin(theta)=r.Q/sin(theta)
    r.Qscatter = r.Q/r.normal.x();
    // Loop over higher orders

    int j=0;
    while (j<r.orders.size() && r.orders[j]*r.Qscatter<2.0*Qmin) j++;
    if (j<r.orders.size() && r.orders[j]*r.Qscatter>=2.0*Qmin) r.lowestDiffOrder=r.orders[j];
    while (j<r.orders.size() && r.orders[j]*r.Qscatter<=2.0*Qmax) {
      r.highestDiffOrder=r.orders[j];
      j++;
    }
  }
  if (r.lowestDiffOrder!=0)
    r.scatteredRay = Projector::normal2scattered(r.normal);
  else
    r.scatteredRay = Vec3D();
}

void Crystal::updateRotation() {
  if (not updateEnabled)
    return;
  UpdateRef updateRef(this);
  for (int i=0; i<reflections.size(); i++) {
    updateRef(reflections[i]);
  }
  emit reflectionsUpdate();
}

int Crystal::reflectionCount() {
  QVector<Reflection> r = getReflectionList();
  return r.size();
}

Reflection Crystal::getReflection(int i) {
  QVector<Reflection> r = getReflectionList();
  if (i<r.size()) {
    return r[i];
  } else {
    return Reflection();
  }
}

Reflection Crystal::getClosestReflection(const Vec3D& normal) {
  QVector<Reflection> r = getReflectionList();
  int minIdx=-1;
  double minDist=0;
  for (int n=r.size(); n--; ) {
    double dist=(r[n].normal-normal).norm_sq();
    if (dist<minDist or minIdx<0) {
      minDist=dist;
      minIdx=n;
    }
  }
  if (minIdx>=0) {
    return r[minIdx];
  } else {
    return Reflection();
  }
}


QVector<Reflection> Crystal::getReflectionList() {
  return reflections;
}

Vec3D Crystal::uvw2Real(const Vec3D& v) {
  return MRot*MReal*v;
}

Vec3D Crystal::uvw2Real(const int u, const int v, const int w) {
  return uvw2Real(Vec3D(u,v,w));
}


Vec3D Crystal::hkl2Reziprocal(const Vec3D& v) {
  return MRot*MReziprocal*v;
}

Vec3D Crystal::hkl2Reziprocal(const int h, const int k, const int l) {
  return hkl2Reziprocal(Vec3D(h,k,l));
}


Mat3D Crystal::getRealOrientationMatrix() const {
  return MReal;
}

Mat3D Crystal::getReziprocalOrientationMatrix() const {
  return MReziprocal;
}

Mat3D Crystal::getRotationMatrix() const {
  return MRot;
}

void Crystal::addProjector(Projector* p) {
  connectedProjectors.addObject(p);
  connect(p, SIGNAL(wavevectorsUpdated()), this, SLOT(updateWavevectorsFromProjectors()));
  connect(p, SIGNAL(markerAdded(AbstractMarkerItem*)), this, SIGNAL(markerAdded(AbstractMarkerItem*)));
  connect(p, SIGNAL(markerChanged(AbstractMarkerItem*)), this, SIGNAL(markerChanged(AbstractMarkerItem*)));
  connect(p, SIGNAL(markerClicked(AbstractMarkerItem*)), this, SIGNAL(markerClicked(AbstractMarkerItem*)));
  connect(p, SIGNAL(markerRemoved(AbstractMarkerItem*)), this, SIGNAL(markerRemoved(AbstractMarkerItem*)));
  emit projectorAdded(p);
  emit fitObjectAdded(p);
  foreach (AbstractMarkerItem* item, p->getAllMarkers())
    emit markerAdded(item);
  updateWavevectorsFromProjectors();
  foreach (Projector* p, getConnectedProjectors())
    p->enableProjection(updateEnabled);
}

void Crystal::removeProjector(Projector* p) {
  foreach (AbstractMarkerItem* item, p->getAllMarkers())
    emit markerRemoved(item);
  connectedProjectors.removeObject(p);
  disconnect(p);
  emit projectorRemoved(p);
  emit fitObjectRemoved(p);
  updateWavevectorsFromProjectors();
}


void Crystal::updateWavevectorsFromProjectors() {
  double hi=0.0;
  double lo=0.0;
  for (int i=0; i<connectedProjectors.size(); i++) {
    Projector* p=dynamic_cast<Projector*>(connectedProjectors.at(i));
    if ((i==0) or (p->Qmin()<lo))
      lo=p->Qmin();
    if ((i==0) or (p->Qmax()*sin(M_PI/360.0*p->TTmax())>hi))
      hi=p->Qmax()*sin(M_PI/360.0*p->TTmax());
  }
  setWavevectors(lo,hi);
}

QList<Projector*> Crystal::getConnectedProjectors() {
  QList<Projector*> list;
  for (int i=0; i<connectedProjectors.size(); i++)
    list << dynamic_cast<Projector*>(connectedProjectors.at(i));
  return list;
}

QList<FitObject*> Crystal::getFitObjects() {
  QList<FitObject*> list;
  list << this;
  foreach (Projector* p, getConnectedProjectors()) {
    list << p;
  }
  return list;
}

QList<AbstractMarkerItem*> Crystal::getMarkers() {
  QList<AbstractMarkerItem*> list;
  foreach (Projector* p, getConnectedProjectors())
    list += p->getAllMarkers();
  return list;
}

void Crystal::setRotationAxis(const Vec3D& axis, RotationAxisType type) {
  if ((axis!=rotationAxis) or (axisType!=type)) {
    rotationAxis=axis;
    axisType=type;
    emit rotationAxisChanged();
  }
}

Vec3D Crystal::getRotationAxis() const {
  return rotationAxis;
}

Vec3D Crystal::getLabSystemRotationAxis() const {
  if (axisType==ReziprocalSpace) {
    Vec3D v(MRot*MReziprocal*rotationAxis);
    v.normalize();
    return v;
  } else if (axisType==DirectSpace) {
    Vec3D v(MRot*MReal*rotationAxis);
    v.normalize();
    return v;
  }
  return rotationAxis.normalized();
}

Crystal::RotationAxisType Crystal::getRotationAxisType() const {
  return axisType;
}

QList<double> Crystal::getCell() {
  QList<double> cell;
  cell << a << b << c << alpha << beta << gamma;
  return cell;
}

Spacegroup* Crystal::getSpacegroup() {
  return &spaceGroup;
}

void Crystal::enableUpdate(bool b) {
  updateEnabled=b;
  foreach (Projector* p, getConnectedProjectors())
    p->enableProjection(updateEnabled);
}

void Crystal::synchronUpdate(bool b) {
  updateIsSynchron=b;
}

void Crystal::calcEulerAngles(double &omega, double &chi, double &phi) {
  omega=-atan2(MRot(0,1),MRot(1,1));
  //chi=asin(MRot[2][1]);
  double s=sin(omega);
  double c=cos(omega);
  if (fabs(c)>fabs(s)) {
    chi=atan2(MRot(2,1), MRot(1,1)/c);
  } else {
    chi=atan2(MRot(2,1), MRot(0,1)/s);
  }
  Mat3D M(Mat3D(Vec3D(1,0,0), -chi)*Mat3D(Vec3D(0,0,1), -omega)*MRot);
  phi=atan2(M(0,2),M(0,0));
}

void Crystal::setEulerAngles(double omega, double chi, double phi) {
  Mat3D M(Vec3D(0,0,1), omega);
  M*=Mat3D(Vec3D(1,0,0), chi);
  M*=Mat3D(Vec3D(0,1,0), phi);
  setRotation(M);
}

void Crystal::slotSetSGConstrains() {
  setCell(getCell());

  QList<int> constrains = spaceGroup.getConstrains();

  int unconstrainedLength=0;
  for (int i=0; i<6; i++) {
    if ((i<3) && (constrains[i]==0)) unconstrainedLength++;
    cellGroup.parameters().at(i)->setChangeable(constrains[i]==0);
  }

  if (unconstrainedLength<2) {
    for (int i=0; i<3; i++) {
      cellGroup.parameters().at(i)->setChangeable(false);
    }
  }
  //emit constrainsChanged();
}



void Crystal::convertRtoH() {
  Vec3D a = uvw2Real( 1,-1, 0);
  //    b = uvw2Real( 0, 1,-1);
  Vec3D c = uvw2Real( 1, 1, 1);
  double a_norm = a.norm();
  double c_norm = c.norm();

  MRot.lmult(VectorPairRotation(MRot*Vec3D(1,0,0), MRot*Vec3D(0,0,1), a/a_norm, c/c_norm));

  setCell(a_norm, a_norm, c_norm, 90, 90, 120);
}

void Crystal::convertHtoR() {
  Vec3D a = uvw2Real( 2, 1, 1)/3;
  Vec3D b = uvw2Real(-1, 1, 1)/3;
  //    c = uvw2Real(-1,-2, 1)/3;
  double l=a.norm();
  double ang=180*M_1_PI*acos(a*b/l/l);

  MRot.lmult(VectorPairRotation(MRot*Vec3D(1,0,0), MRot*Vec3D(cos(M_PI/180*ang), sin(M_PI/180*ang), 0), a/l, b/l));
  setCell(l,l,l,ang,ang,ang);
}


const char XML_Crystal_Element[] = "Crystal";
const char XML_Crystal_Spacegroup[] = "Spacegroup";
const char XML_Crystal_Spacegroup_symbol[] = "symbol";
const char XML_Crystal_Cell[] = "Cell";
const char XML_Crystal_Cell_a[] = "a";
const char XML_Crystal_Cell_b[] = "b";
const char XML_Crystal_Cell_c[] = "c";
const char XML_Crystal_Cell_alpha[] = "alpha";
const char XML_Crystal_Cell_beta[] = "beta";
const char XML_Crystal_Cell_gamma[] = "gamma";
const char XML_Crystal_Orientation[] = "Orientation";
const char XML_Crystal_Orientation_omega[] = "omega";
const char XML_Crystal_Orientation_phi[] = "phi";
const char XML_Crystal_Orientation_chi[] = "chi";
const char XML_Crystal_Rotation[] = "RotationAxis";
const char XML_Crystal_Rotation_x[] = "x";
const char XML_Crystal_Rotation_y[] = "y";
const char XML_Crystal_Rotation_z[] = "z";
const char XML_Crystal_Rotation_type[] = "type";

void Crystal::saveToXML(QDomElement base) {
  QDomDocument doc = base.ownerDocument();
  QDomElement crystalElement = ensureElement(base, XML_Crystal_Element);

  QDomElement e = crystalElement.appendChild(doc.createElement(XML_Crystal_Spacegroup)).toElement();
  e.setAttribute(XML_Crystal_Spacegroup_symbol, spaceGroup.groupSymbol());

  e = crystalElement.appendChild(doc.createElement(XML_Crystal_Cell)).toElement();
  e.setAttribute(XML_Crystal_Cell_a, a);
  e.setAttribute(XML_Crystal_Cell_b, b);
  e.setAttribute(XML_Crystal_Cell_c, c);
  e.setAttribute(XML_Crystal_Cell_alpha, alpha);
  e.setAttribute(XML_Crystal_Cell_beta, beta);
  e.setAttribute(XML_Crystal_Cell_gamma, gamma);

  double omega, chi, phi;
  calcEulerAngles(omega, chi, phi);
  e = crystalElement.appendChild(doc.createElement(XML_Crystal_Orientation)).toElement();
  e.setAttribute(XML_Crystal_Orientation_omega, 180.0*M_1_PI*omega);
  e.setAttribute(XML_Crystal_Orientation_phi, 180.0*M_1_PI*phi);
  e.setAttribute(XML_Crystal_Orientation_chi, 180.0*M_1_PI*chi);

  e = crystalElement.appendChild(doc.createElement(XML_Crystal_Rotation)).toElement();
  e.setAttribute(XML_Crystal_Rotation_x, rotationAxis.x());
  e.setAttribute(XML_Crystal_Rotation_y, rotationAxis.y());
  e.setAttribute(XML_Crystal_Rotation_z, rotationAxis.z());
  e.setAttribute(XML_Crystal_Rotation_type, getRotationAxisType());

}

bool Crystal::loadFromXML(QDomElement base) {
  QDomElement crystalElement = base;
  if (crystalElement.tagName()!=XML_Crystal_Element)
    crystalElement = base.elementsByTagName(XML_Crystal_Element).at(0).toElement();
  if (crystalElement.isNull()) return false;
  for (QDomElement e=crystalElement.firstChildElement(); !e.isNull(); e=e.nextSiblingElement()) {
    bool ok = true;
    if (e.tagName()==XML_Crystal_Spacegroup) {
      if (!spaceGroup.setGroupSymbol(e.attribute(XML_Crystal_Spacegroup_symbol))) return false;
    } else if (e.tagName()==XML_Crystal_Cell) {
      double a = readDouble(e, XML_Crystal_Cell_a, ok);
      double b = readDouble(e, XML_Crystal_Cell_b, ok);
      double c = readDouble(e, XML_Crystal_Cell_c, ok);
      double alpha = readDouble(e, XML_Crystal_Cell_alpha, ok);
      double beta = readDouble(e, XML_Crystal_Cell_beta, ok);
      double gamma = readDouble(e, XML_Crystal_Cell_gamma, ok);
      if (ok) internalSetCell(a,b,c,alpha, beta, gamma);
    } else if (e.tagName()==XML_Crystal_Orientation) {
      double omega = M_PI/180.0*readDouble(e, XML_Crystal_Orientation_omega, ok);
      double chi = M_PI/180.0*readDouble(e, XML_Crystal_Orientation_chi, ok);
      double phi = M_PI/180.0*readDouble(e, XML_Crystal_Orientation_phi, ok);
      if (ok) setEulerAngles(omega, chi, phi);
    } else if (e.tagName()==XML_Crystal_Rotation) {
      double x = readDouble(e, XML_Crystal_Rotation_x, ok);
      double y = readDouble(e, XML_Crystal_Rotation_y, ok);
      double z = readDouble(e, XML_Crystal_Rotation_z, ok);
      int type = readInt(e, XML_Crystal_Rotation_type, ok);
      if (ok) setRotationAxis(Vec3D(x,y,z), static_cast<RotationAxisType>(type));
    }

  }
  return true;
}


Crystal::CellGroup::CellGroup(Crystal* c):
    FitParameterGroup(c),
    crystal(c)
{
  addParameter("a");
  addParameter("b");
  addParameter("c");
  addParameter("alpha");
  addParameter("beta");
  addParameter("gamma");

}

double Crystal::CellGroup::value(int member) const {
  if (member==0) {
    return crystal->a;
  } else if (member==1) {
    return crystal->b;
  } else if (member==2) {
    return crystal->c;
  } else if (member==3) {
    return crystal->alpha;
  } else if (member==4) {
    return crystal->beta;
  } else if (member==5) {
    return crystal->gamma;
  }
  return -1.0;
}

void Crystal::CellGroup::doSetValue(QList<double> values) {
  crystal->internalSetCell(values.at(0),
                           values.at(1),
                           values.at(2),
                           values.at(3),
                           values.at(4),
                           values.at(5));
}

double Crystal::CellGroup::epsilon(int member) const {
  if (member<3) return 0.0001;
  return 0.001;
}

double Crystal::CellGroup::lowerBound(int member) const {
  if (member<3) return 0.5;
  return 10;
}

double Crystal::CellGroup::upperBound(int member) const {
  if (member<3) return 25.0;
  return 170;
}

void Crystal::CellGroup::notifySetEnabled(int member, bool b) {
  // Only check, if the parameter was enabled
  if (b) {
    bool allChecked = true;
    // Check if all changable parameters are checked
    for (int i=0; i<3; i++) {
      FitParameter* q = groupParameters.at(i);
      if (q->isChangeable() && !q->isEnabled()) allChecked = false;
    }
    // if this is the case, disable the next changeable parameter
    if (allChecked) {
      for (int i=0; i<3; i++) {
        FitParameter* q = groupParameters.at((member + i + 1)%3);
        if (q->isChangeable() && q->isEnabled()) {
          q->setEnabled(false);
          return;
        }
      }

    }
  }
}

void Crystal::CellGroup::notifySetChangeable(int member, bool b) {
  if (!b) {
    notifySetEnabled(member, true);
  }
}

Crystal::OrientationGroup::OrientationGroup(Crystal* c):
    FitParameterGroup(c),
    crystal(c)
{
  omega = chi = phi = 0.0;
  addParameter("omega", true);
  addParameter("chi", true);
  addParameter("phi", true);
}

void Crystal::OrientationGroup::setBaseRotation(const Mat3D &R) {
  baseRotation = R;
  omega = chi = phi = 0.0;
}

double Crystal::OrientationGroup::value(int member) const {
  if (member==0) {
    return omega;
  } else if (member==1) {
    return chi;
  } else if (member==2) {
    return phi;
  }
  return -1.0;
}

void Crystal::OrientationGroup::doSetValue(QList<double> values) {
  omega = values.at(0);
  chi   = values.at(1);
  phi   = values.at(2);
  Mat3D M;
  M*=Mat3D(Vec3D(0,0,1), M_PI/180.0*omega);
  M*=Mat3D(Vec3D(1,0,0), M_PI/180.0*chi);
  M*=Mat3D(Vec3D(0,1,0), M_PI/180.0*phi);
  M*= baseRotation;
  crystal->setRotation(M);
}

double Crystal::OrientationGroup::epsilon(int member) const {
  return 0.0001;
}

double Crystal::OrientationGroup::lowerBound(int member) const {
  return -10;
}

double Crystal::OrientationGroup::upperBound(int member) const {
  return 10;
}

void Crystal::saveParametersAsDefault() {
  QSettings settings;
  double omega, chi, phi;
  calcEulerAngles(omega, chi, phi);
  settings.beginGroup("Crystal");
  settings.setValue("Spacegroup", spaceGroup.groupSymbol());
  settings.setValue("CellA", a);
  settings.setValue("CellB", b);
  settings.setValue("CellC", c);
  settings.setValue("CellAlpha", alpha);
  settings.setValue("CellBeta", beta);
  settings.setValue("CellGamma", gamma);
  settings.setValue("EulerOmega", omega);
  settings.setValue("EulerChi", chi);
  settings.setValue("EulerPhi", phi);
  settings.endGroup();
}
