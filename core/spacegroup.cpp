#include "spacegroup.h"

#include <QRegExp>
#include <QStringList>
#include <QDebug>
#include <iomanip>

#include "defs.h"

Spacegroup::SpacegroupSymbolInfo::SpacegroupSymbolInfo(int _ITnr, QString _nrMod, QString _HM, QString _HMmod, QString _Hall) {
  spacegroupNumber = _ITnr;
  spacegroupNumberModifier = _nrMod;
  hermannMauguinSymbol = _HM;
  compactHermannMauguinSymbol = hermannMauguinSymbol;
  compactHermannMauguinSymbol.remove(' ');
  hermannMauguinModifier = _HMmod;
  hallSymbol = _Hall;
}

bool Spacegroup::SpacegroupSymbolInfo::match(QString s) {
  if (s==QString::number(spacegroupNumber))
    return true;
  if (!spacegroupNumberModifier.isEmpty() && s==QString::number(spacegroupNumber)+":"+spacegroupNumberModifier)
    return true;
  QString tmp(s);
  tmp.remove(' ');
  if (tmp==compactHermannMauguinSymbol)
    return true;
  if (!hermannMauguinModifier.isEmpty() && tmp==compactHermannMauguinSymbol+':'+hermannMauguinModifier)
    return true;
  if (s==hallSymbol)
    return true;
  return false;
}

QString Spacegroup::SpacegroupSymbolInfo::HerrmannMauguin() const {
  return hermannMauguinSymbol;
}

QString Spacegroup::SpacegroupSymbolInfo::Hall() const {
  return hallSymbol;
}

Spacegroup::System Spacegroup::SpacegroupSymbolInfo::system() const {
  if (spacegroupNumber<=2) {
    return Spacegroup::triclinic;
  } else if (spacegroupNumber<=15) {
    return Spacegroup::monoclinic;
  } else if (spacegroupNumber<=74) {
    return Spacegroup::orthorhombic;
  } else if (spacegroupNumber<=142) {
    return Spacegroup::tetragonal;
  } else if (spacegroupNumber<=167) {
    return Spacegroup::trigonal;
  } else if (spacegroupNumber<=194) {
    return Spacegroup::hexagonal;
  } else return Spacegroup::cubic;
}


Spacegroup::Spacegroup(QObject* parent): QObject(parent) {
  setGroupSymbol("P1");
}

Spacegroup::Spacegroup(const Spacegroup &o) {
  symbol = o.symbol;
  crystalsystem = o.crystalsystem;
  group = o.group;
  extinctionChecks = o.extinctionChecks;
}

bool Spacegroup::setGroupSymbol(QString s) {
  QList<SpacegroupSymbolInfo>::iterator iter;
  for (iter=groupInfos.begin(); iter!=groupInfos.end(); ++iter) {
    if (iter->match(s)) {

      QList<int> oldConstrains = getConstrains();
      System oldSystem = crystalSystem();

      if (!generateGroup(iter->Hall())) return false;

      symbol = s;
      crystalsystem = iter->system();

      if (oldSystem==crystalSystem() && oldSystem==trigonal && oldConstrains!=getConstrains()) {
        if (oldConstrains.at(3)==0) {
          emit triclinRtoH();
        } else {
          emit triclinHtoR();
        }
      }

      if (getConstrains()!=oldConstrains)
        emit constrainsChanged();

      emit groupChanged();

      return true;
    }
  }
  return false;
}

QString Spacegroup::groupSymbol() const {
  return symbol;
}

Spacegroup::System Spacegroup::crystalSystem() const {
  return crystalsystem;
}

QList<int> Spacegroup::getConstrains() const {
  return constrains;
}

QList< TMat3D<int> > Spacegroup::getPointgroup() const {
  return pointgroup;
}

QList< TMat3D<int> > Spacegroup::getLauegroup() const {
  return lauegroup;
}

Spacegroup::GroupElement::GroupElement(int m11, int m12, int m13, int m21, int m22, int m23, int m31, int m32, int m33, int t1, int t2, int t3): M(m11, m12, m13, m21, m22, m23, m31, m32, m33), t(t1, t2, t3) {
  normalize();
};

Spacegroup::GroupElement::GroupElement(const TMat3D<int> &_M, const TVec3D<int> &_t): M(_M), t(_t) {
  normalize();
};

void Spacegroup::GroupElement::normalize() {
  for (int i=0; i<3; i++) {
    t(i)%=MOD;
    if (t(i)<0)
      t(i)+=MOD;
  }
}

void Spacegroup::GroupElement::print() const {
  QStringList l;
  TMat3D<int> O(M);
  O.transpose();
  l << "x" << "y" << "z";
  for (int i=0; i<3; i++) {
    bool first=true;
    TVec3D<int> v;
    v(i)=1;
    v=O*v;
    for (int j=0; j<3; j++) {
      if (v(j)==1) {
        if (!first) cout << "+";
        cout << qPrintable(l[j]);
        first = false;
      } else if (v(j)==-1) {
        cout << "-" << qPrintable(l[j]);
        first=false;
      }
    }
    if (t(i)!=0) {
      int d = ggt(t(i), MOD);
      cout << "+" << t(i)/d << "/" << MOD/d;
    }
    cout << ", ";
  }
  cout << endl;
}

Spacegroup::GroupElement Spacegroup::GroupElement::operator*(const Spacegroup::GroupElement& o) const {
  return Spacegroup::GroupElement(M*o.M, M*o.t + t);
}

bool Spacegroup::GroupElement::operator==(const Spacegroup::GroupElement& o) const {
  return ((M==o.M) && (t==o.t));
}


bool Spacegroup::isExtinct(const TVec3D<int>& reflection) const {
  for (int i=0; i<extinctionChecks.size(); i++) {
    int s = (reflection*extinctionChecks.at(i).t)%GroupElement::MOD;
    if (s<0) s += GroupElement::MOD;
    if (s!=0) {
      if ((extinctionChecks.at(i).M*reflection).isNull()) {
        return true;
      }
    }
  }
  return false;
}

template <class T> void Spacegroup::addToGroup(QList<T> &group, const T& e) {
  if (!group.contains(e)) {
    group << e;
    //cout << "Generator: ";
    //e.print();
    for (int i=0; i<group.size(); i++) {
      addToGroup(group, e*group[i]);
      addToGroup(group, group[i]*e);
    }
  }
}


bool Spacegroup::generateGroup(QString hall) {
  QList<GroupElement> tmpGroup;


  QRegExp shiftVector("\\s*\\((-?\\d+)\\s+(-?\\d+)\\s+(-?\\d+)\\)$");
  if (shiftVector.indexIn(hall)!=-1) {
    hall.truncate(shiftVector.pos());
  }

  QStringList hallElements = hall.split(' ');

  QRegExp latticeCentering("(-?)([PABCIRSTF])");
  if (!latticeCentering.exactMatch(hallElements.takeFirst())) return false;

  // Identity is always in Pointgroup
  addToGroup(tmpGroup, GroupElement(1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0));

  // Check for inversion
  if (!latticeCentering.capturedTexts().at(1).isEmpty())
    addToGroup(tmpGroup, GroupElement(-1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0));



  QString center = latticeCentering.capturedTexts().at(2);
  if (center=="P") {
    // No centering vectors...
  } else if (center=="A") {
    addToGroup(tmpGroup, GroupElement(1, 0, 0, 0, 1, 0, 0, 0, 1,                     0,   GroupElement::MOD/2,   GroupElement::MOD/2));
  } else if (center=="B") {
    addToGroup(tmpGroup, GroupElement(1, 0, 0, 0, 1, 0, 0, 0, 1,   GroupElement::MOD/2,                     0,   GroupElement::MOD/2));
  } else if (center=="C") {
    addToGroup(tmpGroup, GroupElement(1, 0, 0, 0, 1, 0, 0, 0, 1,   GroupElement::MOD/2,   GroupElement::MOD/2,                     0));
  } else if (center=="I") {
    addToGroup(tmpGroup, GroupElement(1, 0, 0, 0, 1, 0, 0, 0, 1,   GroupElement::MOD/2,   GroupElement::MOD/2,   GroupElement::MOD/2));
  } else if (center=="R") {
    addToGroup(tmpGroup, GroupElement(1, 0, 0, 0, 1, 0, 0, 0, 1, 2*GroupElement::MOD/3,   GroupElement::MOD/3,   GroupElement::MOD/3));
    addToGroup(tmpGroup, GroupElement(1, 0, 0, 0, 1, 0, 0, 0, 1,   GroupElement::MOD/3, 2*GroupElement::MOD/3, 2*GroupElement::MOD/3));
  } else if (center=="S") {
    addToGroup(tmpGroup, GroupElement(1, 0, 0, 0, 1, 0, 0, 0, 1,   GroupElement::MOD/3,   GroupElement::MOD/3, 2*GroupElement::MOD/3));
    addToGroup(tmpGroup, GroupElement(1, 0, 0, 0, 1, 0, 0, 0, 1, 2*GroupElement::MOD/3, 2*GroupElement::MOD/3,   GroupElement::MOD/3));
  } else if (center=="T") {
    addToGroup(tmpGroup, GroupElement(1, 0, 0, 0, 1, 0, 0, 0, 1,   GroupElement::MOD/3, 2*GroupElement::MOD/3,   GroupElement::MOD/3));
    addToGroup(tmpGroup, GroupElement(1, 0, 0, 0, 1, 0, 0, 0, 1, 2*GroupElement::MOD/3,   GroupElement::MOD/3, 2*GroupElement::MOD/3));
  } else if (center=="F") {
    addToGroup(tmpGroup, GroupElement(1, 0, 0, 0, 1, 0, 0, 0, 1,                     0,   GroupElement::MOD/2,   GroupElement::MOD/2));
    addToGroup(tmpGroup, GroupElement(1, 0, 0, 0, 1, 0, 0, 0, 1,   GroupElement::MOD/2,                     0,   GroupElement::MOD/2));
    addToGroup(tmpGroup, GroupElement(1, 0, 0, 0, 1, 0, 0, 0, 1,   GroupElement::MOD/2,   GroupElement::MOD/2,                     0));
  } else {
    cout << "Unknown Centering symbol" << endl;
    return false;
  }

  QString precedingDirection;
  int precedingN=-1;
  for (int n=0; n<hallElements.size(); n++) {
    QRegExp seitz("(-?)([12346])([xyz\"\\*']?)([abcnuvwd]*)([12345]?)");
    if (!seitz.exactMatch(hallElements.at(n))) return false;

    int N = seitz.capturedTexts().at(2).toInt();
    QString direction = seitz.capturedTexts().at(3);
    if (direction.isEmpty() && n==0)
      direction="z";


    TMat3D<int> rotationPart;
    TVec3D<int> translationPart;
    if (N==1) {
      // Rotational part is already unit matix
    } else if ((N==2) && ((direction=="x") || ((direction=="") && (n!=0) && (precedingN==2 || precedingN==4)))) {
      rotationPart = TMat3D<int>( 1, 0, 0, 0,-1, 0, 0, 0,-1);
      translationPart = TVec3D<int>(1, 0, 0);
    } else if ((N==2) && (direction=="y")) {
      rotationPart = TMat3D<int>(-1, 0, 0, 0, 1, 0, 0, 0,-1);
      translationPart = TVec3D<int>(0, 1, 0);
    } else if ((N==2) && (direction=="z")) {
      rotationPart = TMat3D<int>(-1, 0, 0, 0,-1, 0, 0, 0, 1);
      translationPart = TVec3D<int>(0, 0, 1);
    } else if ((N==3) && (direction=="x")) {
      rotationPart = TMat3D<int>( 1, 0, 0, 0, 0,-1, 0, 1,-1);
      translationPart = TVec3D<int>(1, 0, 0);
    } else if ((N==3) && (direction=="y")) {
      rotationPart = TMat3D<int>(-1, 0, 1, 0, 1, 0,-1, 0, 0);
      translationPart = TVec3D<int>(0, 1, 0);
    } else if ((N==3) && (direction=="z")) {
      rotationPart = TMat3D<int>( 0,-1, 0, 1,-1, 0, 0, 0, 1);
      translationPart = TVec3D<int>(0, 0, 1);
    } else if ((N==4) && (direction=="x")) {
      rotationPart = TMat3D<int>( 1, 0, 0, 0, 0,-1, 0, 1, 0);
      translationPart = TVec3D<int>(1, 0, 0);
    } else if ((N==4) && (direction=="y")) {
      rotationPart = TMat3D<int>( 0, 0, 1, 0, 1, 0,-1, 0, 0);
      translationPart = TVec3D<int>(0, 1, 0);
    } else if ((N==4) && (direction=="z")) {
      rotationPart = TMat3D<int>( 0,-1, 0, 1, 0, 0, 0, 0, 1);
      translationPart = TVec3D<int>(0, 0, 1);
    } else if ((N==6) && (direction=="x")) {
      rotationPart = TMat3D<int>( 1, 0, 0, 0, 1,-1, 0, 1, 0);
      translationPart = TVec3D<int>(1, 0, 0);
    } else if ((N==6) && (direction=="y")) {
      rotationPart = TMat3D<int>( 0, 0, 1, 0, 1, 0,-1, 0, 1);
      translationPart = TVec3D<int>(0, 1, 0);
    } else if ((N==6) && (direction=="z")) {
      rotationPart = TMat3D<int>( 1,-1, 0, 1, 0, 0, 0, 0, 1);
      translationPart = TVec3D<int>(0, 0, 1);
    } else if ((N==2) && (direction=="'") && (precedingDirection=="x"))  {
      rotationPart = TMat3D<int>(-1, 0, 0, 0, 0,-1, 0,-1, 0);
      translationPart = TVec3D<int>(0, 1,-1);
    } else if ((N==2) && (direction=="'") && (precedingDirection=="y")) {
      rotationPart = TMat3D<int>( 0, 0,-1, 0,-1, 0,-1, 0, 0);
      translationPart = TVec3D<int>(1, 0,-1);
    } else if ((N==2) && (((direction=="'") && (precedingDirection=="z"))  || ((direction=="") && (n!=0) && (precedingN==3 || precedingN==6)))) {
      rotationPart = TMat3D<int>( 0,-1, 0,-1, 0, 0, 0, 0,-1);
      translationPart = TVec3D<int>(1,-1, 0);
    } else if ((N==2) && (direction=="\"") && (precedingDirection=="x")) {
      rotationPart = TMat3D<int>(-1, 0, 0, 0, 0, 1, 0, 1, 0);
      translationPart = TVec3D<int>(0, 1, 1);
    } else if ((N==2) && (direction=="\"") && (precedingDirection=="y")) {
      rotationPart = TMat3D<int>( 0, 0, 1, 0,-1, 0, 1, 0, 0);
      translationPart = TVec3D<int>(1, 0, 1);
    } else if ((N==2) && (direction=="\"") && (precedingDirection=="z")) {
      rotationPart = TMat3D<int>( 0, 1, 0, 1, 0, 0, 0, 0,-1);
      translationPart = TVec3D<int>(1, 1, 0);
    } else if ((N==3) && ((direction=="*") || ((direction=="") && (n!=0)))) {
      rotationPart = TMat3D<int>( 0, 0, 1, 1, 0, 0, 0, 1, 0);
      translationPart = TVec3D<int>(1, 1, 1);
    } else {
      cout << "can't identify rotation part: " << qPrintable(hallElements.at(n)) << " " << qPrintable(direction) << " " << qPrintable(precedingDirection) << endl;
      return false;
    }

    if (!seitz.capturedTexts().at(1).isEmpty()) {
      rotationPart*=-1;
    }

    int S = 0;
    if (!seitz.capturedTexts().at(5).isEmpty())
      S = seitz.capturedTexts().at(5).toInt();
    if (S>=N) return false;
    translationPart *= GroupElement::MOD * S / N;
    if (seitz.capturedTexts().at(4).contains("a"))
      translationPart += TVec3D<int>(GroupElement::MOD/2, 0, 0);
    if (seitz.capturedTexts().at(4).contains("b"))
      translationPart += TVec3D<int>(0, GroupElement::MOD/2, 0);
    if (seitz.capturedTexts().at(4).contains("c"))
      translationPart += TVec3D<int>(0, 0, GroupElement::MOD/2);
    if (seitz.capturedTexts().at(4).contains("n"))
      translationPart += TVec3D<int>(GroupElement::MOD/2, GroupElement::MOD/2, GroupElement::MOD/2);
    if (seitz.capturedTexts().at(4).contains("u"))
      translationPart += TVec3D<int>(GroupElement::MOD/4, 0, 0);
    if (seitz.capturedTexts().at(4).contains("v"))
      translationPart += TVec3D<int>(0, GroupElement::MOD/4, 0);
    if (seitz.capturedTexts().at(4).contains("w"))
      translationPart += TVec3D<int>(0, 0, GroupElement::MOD/4);
    if (seitz.capturedTexts().at(4).contains("d"))
      translationPart += TVec3D<int>(GroupElement::MOD/4, GroupElement::MOD/4, GroupElement::MOD/4);

    GroupElement e(rotationPart, translationPart);
    cout << "Generator: ";
    e.print();
    addToGroup(tmpGroup, GroupElement(rotationPart, translationPart));
    precedingN = N;
    precedingDirection = direction;
  }
  group = tmpGroup;
  extinctionChecks.clear();
  pointgroup.clear();
  lauegroup.clear();


  for (int i=0; i<group.size(); i++) {
    if (!pointgroup.contains(group.at(i).M))
      pointgroup << group.at(i).M;
    if (!lauegroup.contains(group.at(i).M))
      lauegroup << group.at(i).M;
    if (!group.at(i).t.isNull()) {
      ExtinctionElement e;
      e.M=group.at(i).M-TMat3D<int>();
      e.M.transpose();
      e.t=group.at(i).t;
      extinctionChecks << e;
    }
  }

  addToGroup(lauegroup, TMat3D<int>(-1,0,0,0,-1,0,0,0,-1));

  constrains.clear();
  constrains << 0 << 0 << 0 << 0 << 0 << 0;

  TMat3D<int> metric(2,3,5,3,7,11,5,11,13);
  TMat3D<int> meanMetric(0.0);

  foreach (TMat3D<int> R, pointgroup) {
    meanMetric += R.transposed() * metric * R;
  }

  if (meanMetric(1,1)==meanMetric(0,0)) constrains[1]=-1;
  if (meanMetric(2,2)==meanMetric(1,1)) constrains[2]=-2;
  if (meanMetric(2,2)==meanMetric(0,0)) constrains[2]=-1;

  if (meanMetric(0,2)==meanMetric(1,2)) constrains[4]=-4;
  if (meanMetric(0,1)==meanMetric(0,2)) constrains[5]=-5;
  if (meanMetric(0,1)==meanMetric(1,2)) constrains[5]=-4;

  if (meanMetric(1,2)==0) constrains[3]=90;
  if (meanMetric(0,2)==0) constrains[4]=90;
  if (meanMetric(0,1)==0) constrains[5]=90;

  if (meanMetric(0,0)==meanMetric(1,1) && 2*meanMetric(0,1)==-meanMetric(0,0)) constrains[5]=120;
  if (meanMetric(0,0)==meanMetric(2,2) && 2*meanMetric(0,2)==-meanMetric(0,0)) constrains[4]=120;
  if (meanMetric(1,1)==meanMetric(2,2) && 2*meanMetric(1,2)==-meanMetric(1,1)) constrains[3]=120;


  return true;

}





QList<Spacegroup::SpacegroupSymbolInfo> Spacegroup::groupInfos(Spacegroup::static_init());


