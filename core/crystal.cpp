#include <core/crystal.h>
#include <cmath>
#include <iostream>
#include <core/projector.h>
#include <tools/optimalrotation.h>
#include <core/reflection.h>
#include <core/spacegroup.h>
#include <QtConcurrentRun>

#include <QTime>
#include <QtConcurrentMap>
#include <QMetaObject>

#include "defs.h"
#include "tools/xmltools.h"

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
    QObject(parent),
    FitObject(),
    MReal(),
    MReziprocal(),
    MRot(),
    connectedProjectors(this),
    rotationAxis(1,0,0),
    spaceGroup(this),
    reflections()
{
  spaceGroup.setGroupSymbol("P1");
  internalSetCell(4.0, 4.0, 4.0, 90.0, 90.0, 90.0);
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
  restartReflectionUpdate = false;
  generateReflections();
}

Crystal::Crystal(const Crystal& c) {
  spaceGroup.setGroupSymbol(c.spaceGroup.groupSymbol());
  internalSetCell(c.a,c.b,c.c,c.alpha,c.beta,c.gamma);
  Qmin=c.Qmin;
  Qmax=c.Qmax;
  predictionFactor = 1.0;
  connect(&connectedProjectors, SIGNAL(objectAdded()), this, SLOT(updateWavevectorsFromProjectors()));
  connect(&connectedProjectors, SIGNAL(objectRemoved()), this, SLOT(updateWavevectorsFromProjectors()));

  connect(&spaceGroup, SIGNAL(constrainsChanged()), this, SLOT(slotSetSGConstrains()));
  connect(&spaceGroup, SIGNAL(triclinHtoR()), this, SLOT(convertHtoR()));
  connect(&spaceGroup, SIGNAL(triclinRtoH()), this, SLOT(convertRtoH()));
  connect(&spaceGroup, SIGNAL(groupChanged()),this, SLOT(generateReflections()));
  connect(&reflectionFuture, SIGNAL(finished()), this, SLOT(reflectionGenerated()));

  setRotation(c.getRotationMatrix());
  setRotationAxis(c.getRotationAxis(), c.getRotationAxisType());
  enableUpdate(c.updateEnabled);
  restartReflectionUpdate = false;
}

Crystal::~Crystal() {}



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
  emit cellChanged();
  generateReflections();
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
    Qmax=_Qmax;
    Qmin=_Qmin;
    generateReflections();
  }
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



void Crystal::generateReflections() {
  if (not updateEnabled)
    return;
  if (reflectionFuture.isRunning()) {
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


Crystal::UpdateRef::UpdateRef(Crystal *c) {
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
  emit projectorAdded(p);
}

void Crystal::removeProjector(Projector* p) {
  connectedProjectors.removeObject(p);
  disconnect(p, 0, this, 0);
  emit projectorRemoved(p);
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
  QList<Projector*> r;
  for (int i=0; i<connectedProjectors.size(); i++)
    r << dynamic_cast<Projector*>(connectedProjectors.at(i));
  return r;
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
}

double Crystal::fitParameterValue(int n) {
  if (fitParameterName(n)=="a") {
    return a;
  } else if (fitParameterName(n)=="b") {
    return b;
  } else if (fitParameterName(n)=="c") {
    return c;
  } else if (fitParameterName(n)=="alpha") {
    return alpha;
  } else if (fitParameterName(n)=="beta") {
    return beta;
  } else if (fitParameterName(n)=="gamma") {
    return gamma;
  }
  return 0.0;
}

void Crystal::fitParameterSetValue(int n, double val) {
  if (fitParameterName(n)=="a") {
    setCell(val, b,c,alpha, beta, gamma);
  } else if (fitParameterName(n)=="b") {
    setCell(a, val,c,alpha, beta, gamma);
  } else if (fitParameterName(n)=="c") {
    setCell(a, b,val,alpha, beta, gamma);
  } else if (fitParameterName(n)=="alpha") {
    setCell(a, b,c,val, beta, gamma);
  } else if (fitParameterName(n)=="beta") {
    setCell(a, b,c,alpha, val, gamma);
  } else if (fitParameterName(n)=="gamma") {
    setCell(a, b,c,alpha, beta, val);
  }
}

void Crystal::fitParameterSetEnabled(int n, bool enable) {
  int nDist=0;
  QList<int> spacegroupConstrains = spaceGroup.getConstrains();
  for (int i=0; i<3; i++) {
    if (spacegroupConstrains[i]==0)
      nDist++;
  }
  if (nDist==1)
    nDist=0;
  if ((n<nDist) and enable) {
    bool b=true;
    for (int i=0; i<nDist; i++) {
      b = b and (fitParameterEnabled(i) or i==n);
    }
    if (b) {
      fitParameterSetEnabled((n+1)%nDist, false);
    }
  }
  FitObject::fitParameterSetEnabled(n, enable);
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

  QList<QString> fitParameterTempNames;
  fitParameterTempNames << "b" << "c" << "alpha" << "beta" << "gamma";
  QList<QString> fitParameterNames;
  if (constrains[1]==0 or constrains[2]==0)
    fitParameterNames << "a";
  for (int n=1; n<constrains.size(); n++) {
    if (constrains[n]==0)
      fitParameterNames << fitParameterTempNames[n-1];
  }
  setFitParameterNames(fitParameterNames);
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
