#include <core/crystal.h>
#include <cmath>
#include <iostream>
#include <core/projector.h>
#include <tools/optimalrotation.h>

#include <QTime>
#include <QtConcurrentMap>

using namespace std;



int ggt(int a, int b) {
  while (b) {
    int tb=b;
    b=a%b;
    a=tb;
  }
  return abs(a);
}

class IntIterator {
public:
  IntIterator(int _pos): pos(_pos) {};
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


Crystal::Crystal(QObject* parent=NULL): QObject(parent), FitObject(), MReal(), MReziprocal(), MRot(), reflections(), connectedProjectors(this), rotationAxis(1,0,0) {
  spaceGroup = new SpaceGroup(this);
  spaceGroup->setGroupSymbol("Pm3m");
  internalSetCell(5.0, 5.0, 5.0, 90.0, 90.0, 90.0);
  Qmin=0.0;
  Qmax=1.0;
  predictionFactor = 1.0;
  connect(&connectedProjectors, SIGNAL(objectAdded()), this, SLOT(updateWavevectorsFromProjectors()));
  connect(&connectedProjectors, SIGNAL(objectRemoved()), this, SLOT(updateWavevectorsFromProjectors()));
  connect(spaceGroup, SIGNAL(constrainsChanged()), this, SLOT(slotSetSGConstrains()));
  connect(spaceGroup, SIGNAL(triclinHtoR()), this, SLOT(convertHtoR()));
  connect(spaceGroup, SIGNAL(triclinRtoH()), this, SLOT(convertRtoH()));
  connect(spaceGroup, SIGNAL(groupChanged()),this, SLOT(renewReflections()));
  axisType=LabSystem;
  enableUpdate();
}

Crystal::Crystal(const Crystal& c) {
  cout << "Crystal Copy Constructor" << endl;
  spaceGroup = new SpaceGroup(this);
  spaceGroup->setGroupSymbol(c.spaceGroup->groupSymbol());
  internalSetCell(c.a,c.b,c.c,c.alpha,c.beta,c.gamma);
  Qmin=c.Qmin;
  Qmax=c.Qmax;
  predictionFactor = 1.0;
  connect(&connectedProjectors, SIGNAL(objectAdded()), this, SLOT(updateWavevectorsFromProjectors()));
  connect(&connectedProjectors, SIGNAL(objectRemoved()), this, SLOT(updateWavevectorsFromProjectors()));

  setRotation(c.getRotationMatrix());
  setRotationAxis(c.getRotationAxis(), c.getRotationAxisType());
  enableUpdate(c.updateEnabled);
}

Crystal::~Crystal() {}



void Crystal::setCell(double _a, double _b, double _c, double _alpha, double _beta, double _gamma) {
  QList<double> c;
  c << _a << _b << _c << _alpha << _beta << _gamma;
  setCell(c);
}

void Crystal::setCell(QList<double> cell) {
  if (cell.size()!=6) return;
  QList<int> constrains = spaceGroup->getConstrains();
  for (int i=0; i<6; i++) {
    if (constrains[i]>0) cell[i]=constrains[i];
    if (constrains[i]<0) cell[i]=cell[-constrains[i]-1];
  }

  double _a = cell[0];
  double _b = cell[1];
  double _c = cell[2];
  double _alpha = cell[3];
  double _beta = cell[4];
  double _gamma = cell[5];

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

  // Clear old reflections, will be automatically regenerated if needed
  reflections.clear();

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
  astar=MReziprocal[0];
  bstar=MReziprocal[1];
  cstar=MReziprocal[2];
  emit cellChanged();
  emit reflectionsUpdate();
}

void Crystal::renewReflections() {
  reflections.clear();
  emit reflectionsUpdate();
}

void Crystal::addRotation(const Vec3D& axis, double angle) {
  addRotation(Mat3D(axis, angle));
}

void Crystal::addRotation(const Mat3D& M) {
#ifdef __DEBUG__
  cout << "AddRotation" << endl;
#endif
  MRot.lmult(M);
  MRot.orthogonalize();
  updateRotation();
  emit orientationChanged();
  emit reflectionsUpdate();
}

void Crystal::setRotation(const Mat3D& M) {
#ifdef __DEBUG__
  cout << "setRotation" << endl;
#endif
  MRot = M;
  updateRotation();
  emit orientationChanged();
  emit reflectionsUpdate();
}

void Crystal::setWavevectors(double _Qmin, double _Qmax) {
  if ((_Qmin<_Qmax) and ((_Qmin!=Qmin) or (_Qmax!=Qmax))) {
    Qmax=_Qmax;
    Qmin=_Qmin;
    reflections.clear();
    emit reflectionsUpdate();
  }
}

Crystal::GenerateReflection::GenerateReflection(Crystal *_c, int _hMax):
    c(_c),
    hMax(_hMax),
    aktualH(-_hMax) {
  setAutoDelete(true);
  reflectionNumber = 0;
}

Crystal::GenerateReflection::~GenerateReflection() {
  c->reflections.resize(reflectionNumber);
}

void Crystal::GenerateReflection::run() {
  QThreadPool::globalInstance()->tryStart(this);

  Crystal::UpdateRef updateRef(c);

  int h;

  while ((h=aktualH.fetchAndAddRelaxed(1))<=hMax) {

    //|h*as+k*bs|^2=h^2*|as|^2+k^2*|bs|^2+2*h*k*as*bs==(2*Qmax)^2
    // k^2 +2*k*h*as*bs/|bs|^2 + (h^2*|as|^2-4*Qmax^2)/|bs|^2 == 0
    double ns = 1.0/c->bstar.norm_sq();
    double p = c->astar*c->bstar*ns*h;
    double q1 = c->astar.norm_sq()*ns*h*h;
    double q2 = M_1_PI*M_1_PI*ns*c->Qmax*c->Qmax;
    double s = p*p-q1+q2;
    int kMin = (s>0)?int(-p-sqrt(s)):0;
    int kMax = (s>0)?int(-p+sqrt(s)):0;

    for (int k=kMin; k<=kMax; k++) {

      int hk_ggt = ggt(h,k);

      Vec3D v = c->MReziprocal*Vec3D(h,k,0);
      ns = 1.0/c->cstar.norm_sq();
      p = v*c->cstar*ns;
      q1 = v.norm_sq()*ns;
      q2 = M_1_PI*M_1_PI*ns*c->Qmax*c->Qmax;
      s = p*p-q1+q2;
      int lMin = (s>0)?int(-p-sqrt(s)):0;
      int lMax = (s>0)?int(-p+sqrt(s)):0;

      for (int l=lMin; l<=lMax; l++) {
        // store only lowest order reflections
        if (ggt(hk_ggt, l)==1) {
          v=c->MReziprocal*Vec3D(h,k,l);
          double Q = 2.0*M_PI*v.norm();

          Reflection r;
          r.h=h;
          r.k=k;
          r.l=l;
          r.hklSqSum=h*h+k*k+l*l;
          r.Q=Q;
          r.d = 2.0*M_PI/Q;
          for (int i=1; i<=int(M_1_PI*c->Qmax*r.d+0.9); i++) {
            if (!c->spaceGroup->isExtinct(TVec3D<int>(i*h, i*k, i*l)))
              r.orders.push_back(i);
          }
          if (r.orders.size()>0) {
            r.normalLocal=v*r.d;
            updateRef(r);
            mutex.lock();
            if (reflectionNumber>=c->reflections.size())
              c->reflections.resize(int(1.2*reflectionNumber));
            c->reflections[reflectionNumber] = r;
            reflectionNumber++;
            mutex.unlock();
          }
        }
      }
    }
  }
}



void Crystal::generateReflections() {
  if (not updateEnabled)
    return;
  // Qmax =2*pi/lambda_min
  // n*lambda=2*d*sin(theta) => n_max=2*d/lambda = Qmax*d/pi
  int hMax = int(M_1_PI*a*Qmax);

  // predicted number of reflections
  double prediction = 4.0/3.0*M_1_PI*M_1_PI*Qmax*Qmax*Qmax*MReal.det();


  reflections.resize(int(1.1*predictionFactor*prediction));
  QTime t1(QTime::currentTime());
  //reflections = QtConcurrent::blockingMappedReduced(IntIterator(-hMax), IntIterator(hMax+1), GenerateReflection(this), Reduce);
  QThreadPool::globalInstance()->start(new GenerateReflection(this, hMax));
  QThreadPool::globalInstance()->waitForDone();

  int N = reflections.size();

  predictionFactor *= 0.8;
  predictionFactor += 0.2*(N/prediction);

  int dt = t1.msecsTo(QTime::currentTime());
  emit info(QString("%1 reflections generated in %2ms (%3 r/ms)  %4 %5 %6 %7").arg(N).arg(dt).arg(1.0*N/dt).arg(predictionFactor*prediction).arg(N/predictionFactor/prediction).arg(MReziprocal.det()).arg(predictionFactor), 0);

}


void Crystal::UpdateRef::operator()(Reflection &r) {
  r.normal=c->MRot*r.normalLocal;
  r.lowestDiffOrder=0;
  r.highestDiffOrder=0;

  // sin(theta) = v*e_x = v.x
  // x direction points toward source, z points upwards
  if (r.normal.x()>0.0) {
    //Q=2*pi/d/sin(theta)=r.Q/sin(theta)
    r.Qscatter = r.Q/r.normal.x();
    // Loop over higher orders

    int j=0;
    while (j<r.orders.size() && r.orders[j]*r.Qscatter<2.0*c->Qmin) j++;
    if (j<r.orders.size() && r.orders[j]*r.Qscatter>=2.0*c->Qmin) r.lowestDiffOrder=r.orders[j];
    while (j<r.orders.size() && r.orders[j]*r.Qscatter<=2.0*c->Qmax) {
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

  QtConcurrent:: blockingMap(reflections, UpdateRef(this));
  Vec3D v = hkl2Reziprocal(1,0,0);
  cout << "(100) = " << v.x() << " " << v.y() << " " << v.z() << endl;
  v = hkl2Reziprocal(-1,0,0);
  cout << "(-100) = " << v.x() << " " << v.y() << " " << v.z() << endl;
  v = hkl2Reziprocal(0,1,0);
  cout << "(010) = " << v.x() << " " << v.y() << " " << v.z() << endl;
  v = hkl2Reziprocal(0,-1,0);
  cout << "(0-10) = " << v.x() << " " << v.y() << " " << v.z() << endl;
  v = hkl2Reziprocal(0,0,1);
  cout << "(001) = " << v.x() << " " << v.y() << " " << v.z() << endl;
  v = hkl2Reziprocal(0,0,-1);
  cout << "(00-1) = " << v.x() << " " << v.y() << " " << v.z() << endl;
  cout << endl;
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
  if (reflections.empty())
    generateReflections();
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
}

void Crystal::removeProjector(Projector* p) {
  connectedProjectors.removeObject(p);
  disconnect(p, 0, this, 0);
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

Vec3D Crystal::getLabSystamRotationAxis() const {
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

SpaceGroup* Crystal::getSpacegroup() {
  return spaceGroup;
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
  QList<int> spacegroupConstrains = spaceGroup->getConstrains();
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

  QList<int> constrains = spaceGroup->getConstrains();

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
  Vec3D a = uvw2Real(1, -1, 0);
  // Vec3D b = uvw2Real(0, 1, -1);
  Vec3D c = uvw2Real(1, 1, 1);
  setCell(a.norm(), a.norm(), c.norm(), 90, 90, 120);
  OptimalRotation r;
  r.addVectorPair(a, uvw2Real(1,0,0));
  r.addVectorPair(c, uvw2Real(0,0,1));
  addRotation(r.getOptimalRotation());

}

void Crystal::convertHtoR() {
  Vec3D a=uvw2Real(2,1,1)/3;
  Vec3D b=uvw2Real(-1,1,1)/3;
  double l=a.norm();
  double ang=180*M_1_PI*acos(a*b/l/l);
  setCell(l,l,l,ang,ang,ang);

  OptimalRotation r;
  r.addVectorPair(a, uvw2Real(1,0,0));
  r.addVectorPair(b, uvw2Real(0,1,0));
  addRotation(r.getOptimalRotation());
}

