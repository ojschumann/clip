#include <Indexer.h>
#include <QtCore/QThreadPool>
#include <QtCore/QMutexLocker>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <QtCore/QtAlgorithms>

#include <vec3D.h>
#include <mat3D.h>



using namespace std;

Indexer::Indexer(QObject* parent): QAbstractTableModel(parent) {
}

int Indexer::rowCount(const QModelIndex & parent) const {
  return solution.count();
}

int Indexer::columnCount(const QModelIndex & parent) const {
  return 3;
}

QVariant Indexer::data(const QModelIndex & index, int role) const {
  if (role==Qt::DisplayRole) {
    if (index.column()==0) {
      return QVariant(solution.at(index.row()).spatialDeviationSum());
    } else if (index.column()==1) {
      return QVariant(solution.at(index.row()).angularDeviationSum());
    } else if (index.column()==2) {
      return QVariant(solution.at(index.row()).hklDeviationSum());
    }
  }
  return QVariant();
}

QVariant Indexer::headerData(int section, Qt::Orientation orientation, int role) const {
  if (role==Qt::DisplayRole) {
    if (orientation==Qt::Horizontal) {
      if (section==0) {
        return QVariant("Spacial");
      } else if (section==1) {
        return QVariant("Angular");
      } else if (section==2) {
        return QVariant("HKL");
      }
    } else {
      return QVariant(section+1);
    }
  }
  return QVariant();
}


void Indexer::sort(int column, Qt::SortOrder order) {
  sortColumn=column;
  sortOrder=order;
  qSort(solution.begin(), solution.end(), SolSort(sortColumn, sortOrder));
  reset();
}

void Indexer::startIndexing(Indexer::IndexingParameter& _p) {
  emit stopWorker();
  p=_p;
  cout << p.pointGroup.size() << endl;
  solution.clear();
  reset();
  IndexWorker* worker=new IndexWorker(p);
  qRegisterMetaType<Solution>();
  connect(worker, SIGNAL(publishSolution(Solution)), this, SLOT(addSolution(Solution)));
  connect(worker, SIGNAL(progressInfo(int, int)), this, SIGNAL(progressInfo(int,int)));
  connect(worker, SIGNAL(destroyed()), this, SLOT(threadFinished()));
  connect(this, SIGNAL(stopWorker()), worker, SLOT(stop()));
  connect(this, SIGNAL(destroyed()), worker, SLOT(stop()));
  QThreadPool::globalInstance()->start(worker);
  emit runningStateChanged(true);
}

void Indexer::addSolution(Solution s) {
  QList<Solution>::iterator iter=qLowerBound(solution.begin(), solution.end(), s, SolSort(sortColumn, sortOrder));
  int n=iter-solution.begin();
  beginInsertRows(QModelIndex(),n,n);
  solution.insert(iter,s);
  endInsertRows();
}

Solution Indexer::getSolution(unsigned int n) {
  return solution[n];
}

void Indexer::threadFinished() {
  emit runningStateChanged(false);
}


Indexer::SolSort::SolSort(int col, Qt::SortOrder order) {
  sortColumn=col;
  sortOrder=order;
};

bool Indexer::SolSort::operator()(const Solution& s1,const Solution& s2) {
  bool b=true;
  if (sortColumn==0) {
    b=s1.angularDeviationSum()<s2.angularDeviationSum();
  } else if (sortColumn==1) {
    b=s1.spatialDeviationSum()<s2.spatialDeviationSum();
  } else if (sortColumn==2) {
    b=s1.hklDeviationSum()<s2.hklDeviationSum();
  }
  if (sortOrder==Qt::DescendingOrder)
    b=not b;
  return b;
}













IndexWorker::IndexWorker(Indexer::IndexingParameter &_p): QObject(), QRunnable(), indexMutex(), angles(), solRotLock(), solutionRotations() {
  indexI=1;
  indexJ=0;
  nextProgressSignal=0;

  isInitiatingThread=true;
  shouldStop=false;
  p=_p;
  OMatInv=p.orientationMatrix.inverse();

  for (unsigned int i=p.markerNormals.size(); i--; ) {
    for (unsigned int j=i; j--; ) {
      AngleInfo info;
      info.index1=i;
      info.index2=j;
      info.cosAng=p.markerNormals.at(i)*p.markerNormals.at(j);
      double c=acos(info.cosAng);
      double c1=cos(c+p.maxAngularDeviation);
      double c2=cos(c-p.maxAngularDeviation);
      info.lowerBound=(c1<c2)?c1:c2;
      info.upperBound=(c1>c2)?c1:c2;
      angles.append(info);
    }
  }
  qSort(angles);
}

void IndexWorker::run() {
  if (isInitiatingThread) {
    isInitiatingThread=false;
    unsigned int count=0;
    while (QThreadPool::globalInstance()->tryStart(this)) {
      count++;
    }
    cout << "Started " << count << " additional threads" << endl;
  }
  int i, j;
  AngleInfo lower;

  while (nextWork(i,j)) {
    double cosAng=p.refs.at(i).normalLocal*p.refs.at(j).normalLocal;
    lower.upperBound=cosAng;
    QList<IndexWorker::AngleInfo>::iterator iter=qLowerBound(angles.begin(), angles.end(), lower, IndexWorker::AngleInfo::cmpAngleInfoUpperBound);
    bool ok=false;
    while (iter!=angles.end() && cosAng>=iter->lowerBound) {
      if (iter->lowerBound<=cosAng && cosAng<iter->upperBound) {
        checkGuess(p.refs.at(i), p.refs.at(j),  *iter);
        checkGuess(p.refs.at(j), p.refs.at(i),  *iter);
      }
      iter++;
    }
  }
}

Mat3D bestRotation(Mat3D M) {
  Mat3D L,R;
  M.svd(L,R);
  double d=L.det()*R.det();
  if (d<0.0) {
    Mat3D T;
    *T.at(2,2)=-1.0;
    return L*T*R;
  } else {
    return L*R;
  }
};

#define MAX(x,y) (((x)>(y))?(x):(y))

void IndexWorker::checkGuess(const Reflection &c1, const Reflection &c2,  const AngleInfo &a) {
  // Prepare Best Rotation Matrix from c1,c2 -> a(1) a(2)
  Mat3D R=c1.normalLocal^p.markerNormals[a.index1];
  R+=(c2.normalLocal^p.markerNormals[a.index2]);

  R=bestRotation(R);


  // Try Indexation of missing reflexions
  Solution s;
  s.indexingRotation=R;

  for (unsigned int n=0; n<p.markerNormals.size(); n++) {
    SolutionItem si;
    si.initialIndexed=true;
#ifdef __DEBUG__
    si.rotatedMarker=R*p.markerNormals.at(n);
#endif
    bool ok=true;
    if (n==a.index1) {
      si.h=c1.h;
      si.k=c1.k;
      si.l=c1.l;
    } else if (n==a.index2) {
      si.h=c2.h;
      si.k=c2.k;
      si.l=c2.l;
    } else {
      Vec3D hklVect(OMatInv*R*p.markerNormals.at(n));
      double max=MAX(MAX(fabs(hklVect[0]),fabs(hklVect[1])),fabs(hklVect[2]));
      hklVect*=1.0/max;
      si.initialIndexed=false;

      for (unsigned int order=1; order<=p.maxOrder; order++) {
        Vec3D t(hklVect*order);
#ifdef __DEBUG__
        si.rationalHkl=t;
#endif
        ok=true;
        for (unsigned int i=3; i--; ) {
          if (fabs(fabs(t[i])-round(fabs(t[i])))>p.maxIntegerDeviation) {
            ok=false;
            break;
          }
        }
        if (ok) {
          si.h=(int)round(t[0]);
          si.k=(int)round(t[1]);
          si.l=(int)round(t[2]);
          break;
        }
      }
    }
    if (ok) {
      s.items.append(si);
    } else {
      break;
    }
  }

  if (p.markerNormals.size()==s.items.size()) {
    // yes, we have a solution!!!
    R*=0.0;
    for (unsigned int n=s.items.size(); n--; ) {
      SolutionItem& si=s.items[n];
      Vec3D v(si.h, si.k, si.l);
      v=p.orientationMatrix*v;
      v.normalize();
      si.latticeVector=v;
      R+=v^p.markerNormals[n];
    }

    s.bestRotation=bestRotation(R);

    if (newSolution(s.bestRotation)) {
      for (unsigned int n=s.items.size(); n--; ) {
        SolutionItem& si=s.items[n];
        si.rotatedMarker=s.bestRotation*p.markerNormals.at(n);
        otimizeScale(si);
      }
      emit publishSolution(s);
    }
  }

}

void IndexWorker::otimizeScale(SolutionItem& si) {
  Vec3D hkl(si.h,si.k,si.l);
  si.rationalHkl=OMatInv*si.rotatedMarker;
  si.rationalHkl*=hkl*si.rationalHkl/si.rationalHkl.norm_sq();
  //sum (hkl-scale*rhkl)^2 = min
  // dsum/scale = 2sum (hkl_i-s*rhkl_i)*rhkl_i == 0!
  // => s* sum( rhkl_i^2 ) = sum ( rhkl_i * hkl_i )
}

bool IndexWorker::newSolution(const Mat3D& M) {
  //TODO: This is possibly a performance lock. The threads might serialize here
  QMutexLocker lock(&solRotLock);
  Mat3D T1(M.transposed());
  for (unsigned int n=solutionRotations.size(); n--; ) {
    Mat3D T2(solutionRotations.at(n)*T1);
    for (unsigned int m=p.pointGroup.size(); m--; ) {
      Mat3D T3(T2-p.pointGroup.at(m));
      if (T3.sqSum()<1e-10) {
        return false;
      }
    }
  }
  solutionRotations.append(M);
  return true;
}


bool IndexWorker::nextWork(int &i, int &j) {
  QMutexLocker lock(&indexMutex);
  if (indexI>=p.refs.size() or shouldStop)
    return false;

  i=indexI;
  j=indexJ;

  if (nextProgressSignal==0) {
    int maxN=p.refs.size()*(p.refs.size()-1)/2;
    int actN=i*(i-1)/2+j;
    nextProgressSignal=maxN/1000;
    emit progressInfo(maxN, actN);
  }
  nextProgressSignal--;
  indexJ++;
  if (indexJ==indexI) {
    indexI++;
    indexJ=0;
  }
  return true;
}    

bool IndexWorker::AngleInfo::operator<(const AngleInfo& o) const {
  return cosAng<o.cosAng;
}

bool IndexWorker::AngleInfo::cmpAngleInfoLowerBound(const AngleInfo &a1, const AngleInfo &a2) {
  return a1.lowerBound<a2.lowerBound;
}
bool IndexWorker::AngleInfo::cmpAngleInfoUpperBound(const AngleInfo &a1, const AngleInfo &a2) {
  return a1.upperBound<a2.upperBound;
}

void IndexWorker::stop() {
  QMutexLocker lock(&indexMutex);
  shouldStop=true;
}
