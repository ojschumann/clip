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
  for (int i=0; i<groupInfos.size(); i++) {
    //cout << i << "/" << groupInfos.size() << " " << qPrintable(groupInfos.at(i).HerrmannMauguin()) << "    " << qPrintable(groupInfos.at(i).Hall()) << endl;
    if (!generateGroup(groupInfos.at(i).Hall())) cout << " " << qPrintable(groupInfos.at(i).HerrmannMauguin()) << "    " << qPrintable(groupInfos.at(i).Hall()) << endl;
  }
  cout << "done" << endl;
}

Spacegroup::Spacegroup(const Spacegroup &o) {
  symbol = o.symbol;
  symbolElements = o.symbolElements;
  crystalsystem = o.crystalsystem;
  group = o.group;
  extinctionChecks = o.extinctionChecks;
}

bool Spacegroup::setGroupSymbol(QString s) {
  QList<SpacegroupSymbolInfo>::iterator iter;
  for (iter=groupInfos.begin(); iter!=groupInfos.end(); ++iter) {
    if (iter->match(s)) {

      if (!generateGroup(iter->Hall())) return false;
      return false;
      bool SystemChg = crystalsystem!=iter->system();
      bool startup = false;
      QString CenterChg;
      if (symbolElements.empty()) {
        startup = true;
      } else {
        CenterChg = symbolElements.first();
      }

      symbol = s;
      crystalsystem = iter->system();
      //symbolElements = iter->elements();
      CenterChg += symbolElements.first();

      bool TriclinSettingChanged = false;
      if ((crystalsystem==trigonal) && (!SystemChg)) {
        if ((CenterChg=="HR") || (CenterChg=="PR")) {
          emit triclinHtoR();
          TriclinSettingChanged=true;
        }
        if ((CenterChg=="RH") || (CenterChg=="RP")) {
          emit triclinRtoH();
          TriclinSettingChanged=true;
        }
      }
      if (startup || SystemChg || TriclinSettingChanged) {
        emit constrainsChanged();
      }
      //generateGroup();
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
  QList<int> r;
  if (crystalsystem==triclinic) {
    r << 0 << 0 << 0 << 0 << 0 << 0;
  } else if (crystalsystem==monoclinic) {
    r << 0 << 0 << 0 << 90 << 0 << 90;
  } else if (crystalsystem==orthorhombic) {
    r << 0 << 0 << 0 << 90 << 90 << 90;
  } else if (crystalsystem==tetragonal) {
    r << 0 << -1 << 0 << 90 << 90 << 90;
  } else if (crystalsystem==trigonal) {
    if (this->symbolElements[0]=="R") {
      r << 0 << -1 << -1 << 0 << -4 << -4;
    } else {
      r << 0 << -1 << 0 << 90 << 90 << 120;
    }
  } else if (crystalsystem==hexagonal) {
    r << 0 << -1 << 0 << 90 << 90 << 120;
  } else if (crystalsystem==cubic) {
    r << 0 << -1 << -1 << 90 << 90 << 90;
  }
  return r;
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


bool Spacegroup::isExtinct(const TVec3D<int>& reflection) {
  for (int i=0; i<extinctionChecks.size(); i++) {
    int s = reflection*extinctionChecks.at(i).t;
    if ((s%GroupElement::MOD)!=0) {
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
  group.clear();
  extinctionChecks.clear();


  QRegExp shiftVector("\\s*\\((-?\\d+)\\s+(-?\\d+)\\s+(-?\\d+)\\)$");
  if (shiftVector.indexIn(hall)!=-1) {
    hall.truncate(shiftVector.pos());
  }

  QStringList hallElements = hall.split(' ');

  QRegExp latticeCentering("(-?)([PABCIRSTF])");
  if (!latticeCentering.exactMatch(hallElements.takeFirst())) return false;

  // Identity is always in Pointgroup
  addToGroup(group, GroupElement(1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0));

  // Check for inversion
  if (!latticeCentering.capturedTexts().at(1).isEmpty())
    addToGroup(group, GroupElement(-1, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0));

  QString center = latticeCentering.capturedTexts().at(2);
  if (center=="P") {
    // No centering vectors...
  } else if (center=="A") {
    addToGroup(group, GroupElement(1, 0, 0, 0, 1, 0, 0, 0, 1,                     0,   GroupElement::MOD/2,   GroupElement::MOD/2));
  } else if (center=="B") {
    addToGroup(group, GroupElement(1, 0, 0, 0, 1, 0, 0, 0, 1,   GroupElement::MOD/2,                     0,   GroupElement::MOD/2));
  } else if (center=="C") {
    addToGroup(group, GroupElement(1, 0, 0, 0, 1, 0, 0, 0, 1,   GroupElement::MOD/2,   GroupElement::MOD/2,                     0));
  } else if (center=="I") {
    addToGroup(group, GroupElement(1, 0, 0, 0, 1, 0, 0, 0, 1,   GroupElement::MOD/2,   GroupElement::MOD/2,   GroupElement::MOD/2));
  } else if (center=="R") {
    addToGroup(group, GroupElement(1, 0, 0, 0, 1, 0, 0, 0, 1, 2*GroupElement::MOD/3,   GroupElement::MOD/3,   GroupElement::MOD/3));
    addToGroup(group, GroupElement(1, 0, 0, 0, 1, 0, 0, 0, 1,   GroupElement::MOD/3, 2*GroupElement::MOD/3, 2*GroupElement::MOD/3));
  } else if (center=="S") {
    addToGroup(group, GroupElement(1, 0, 0, 0, 1, 0, 0, 0, 1,   GroupElement::MOD/3,   GroupElement::MOD/3, 2*GroupElement::MOD/3));
    addToGroup(group, GroupElement(1, 0, 0, 0, 1, 0, 0, 0, 1, 2*GroupElement::MOD/3, 2*GroupElement::MOD/3,   GroupElement::MOD/3));
  } else if (center=="T") {
    addToGroup(group, GroupElement(1, 0, 0, 0, 1, 0, 0, 0, 1,   GroupElement::MOD/3, 2*GroupElement::MOD/3,   GroupElement::MOD/3));
    addToGroup(group, GroupElement(1, 0, 0, 0, 1, 0, 0, 0, 1, 2*GroupElement::MOD/3,   GroupElement::MOD/3, 2*GroupElement::MOD/3));
  } else if (center=="F") {
    addToGroup(group, GroupElement(1, 0, 0, 0, 1, 0, 0, 0, 1,                     0,   GroupElement::MOD/2,   GroupElement::MOD/2));
    addToGroup(group, GroupElement(1, 0, 0, 0, 1, 0, 0, 0, 1,   GroupElement::MOD/2,                     0,   GroupElement::MOD/2));
    addToGroup(group, GroupElement(1, 0, 0, 0, 1, 0, 0, 0, 1,   GroupElement::MOD/2,   GroupElement::MOD/2,                     0));
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
    if (direction.isEmpty()) {
      if (n==0) {
        direction = "z";
      } else if (n==1) {
        if ((precedingN==2) || (precedingN==4)) {
          direction = "x";
        } else if ((precedingN==3) || (precedingN==6)) {
          direction = "'";
        }
      } else direction = "*";
    }

    TMat3D<int> rotationPart;
    TVec3D<int> translationPart;
    if (N==1) {
      // Rotational part is already unit matix
    } else if ((N==2) && (direction=="x")) {
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
    } else if ((N==2) && (direction=="'") && (precedingDirection=="x")) {
      rotationPart = TMat3D<int>(-1, 0, 0, 0, 0,-1, 0,-1, 0);
      translationPart = TVec3D<int>(0, 1,-1);
    } else if ((N==2) && (direction=="'") && (precedingDirection=="y")) {
      rotationPart = TMat3D<int>( 0, 0,-1, 0,-1, 0,-1, 0, 0);
      translationPart = TVec3D<int>(1, 0,-1);
    } else if ((N==2) && (direction=="'") && (precedingDirection=="z")) {
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
    } else if ((N==3) && (direction=="*")) {
      rotationPart = TMat3D<int>( 0, 0, 1, 1, 0, 0, 0, 1, 0);
      translationPart = TVec3D<int>(1, 1, 1);
    } else {
      cout << "can't identify rotation part: " << qPrintable(direction) << " " << N << endl;
      return false;
    }

    if (!seitz.capturedTexts().at(1).isEmpty())
      rotationPart*=-1;

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
    //cout << "Generator ";
    //e.print();
    addToGroup(group, e);
    precedingN = N;
    precedingDirection = direction;
  }


  //cout <<  " Groupsize: " << group.size() << endl;

  for (int i=0; i<group.size(); i++) {
    if (!group.at(i).t.isNull()) {
      ExtinctionElement e;
      e.M=TMat3D<int>() - group.at(i).M;
      e.t=group.at(i).t;
      extinctionChecks << e;
    }
  }
  return true;

}




QList<Spacegroup::SpacegroupSymbolInfo> Spacegroup::groupInfos(Spacegroup::static_init());

/*
    QStringList g;

    g << "P1" << "P-1" << "P2" << "P21" << "C2" << "Pm" << "Pc";
    g << "Cm" << "Cc"<< "P2/m" << "P21/m" << "C2/m" << "P2/c";
    g << "P21/c" << "C2/c" << "P222" << "P2221" << "P21212";
    g << "P212121" << "C2221" << "C222" << "F222" << "I222" << "I212121";
    g << "Pmm2" << "Pmc21" << "Pcc2" << "Pma2" << "Pca21" << "Pnc2";
    g << "Pmn21" << "Pba2" << "Pna21" << "Pnn2" << "Cmm2" << "Cmc21";
    g << "Ccc2" << "Amm2" << "Aem2" << "Ama2" << "Aea2" << "Fmm2";
    g << "Fdd2" << "Imm2" << "Iba2" << "Ima2" << "Pmmm" << "Pnnn";
    g << "Pccm" << "Pban" << "Pmma" << "Pnna" << "Pmna" << "Pcca";
    g << "Pbam" << "Pccn" << "Pbcm" << "Pnnm" << "Pmmn";
    g << "Pbcn" << "Pbca" << "Pnma" << "Cmcm" << "Cmce";
    g << "Cmmm" << "Cccm" << "Cmme" << "Ccce" << "Fmmm";
    g << "Fddd" << "Immm" << "Ibam" << "Ibca" << "Imma";
    g << "P4" << "P41" << "P42" << "P43" << "I4" << "I41" << "P-4";
    g << "I-4" << "P4/m" << "P42/m" << "P4/n" << "P42/n" << "I4/m";
    g << "I41/a" << "P422" << "P4212" << "P4122" << "P41212";
    g << "P4222" << "P42212" << "P4322" << "P43212" << "I422";
    g << "I4122" << "P4mm" << "P4bm" << "P42cm" << "P42nm";
    g << "P4cc" << "P4nc" << "P42mc" << "P42bc" << "I4mm" << "I4cm";
    g << "I41md" << "I41cd" << "P-42m" << "P-42c" << "P-421m" << "P-421c";
    g << "P-4m2" << "P-4c2" << "P-4b2" << "P-4n2" << "I-4m2" << "I-4c2";
    g << "I-42m" << "I-42d" << "P4/mmm" << "P4/mcc" << "P4/nbm" << "P4/nnc";
    g << "P4/mbm" << "P4/mnc" << "P4/nmm" << "P4/ncc" << "P42/mmc" << "P42/mcm";
    g << "P42/nbc" << "P42/nnm" << "P42/mbc" << "P42/mnm" << "P42/nmc";
    g << "P42/ncm" << "I4/mmm" << "I4/mcm" << "I41/amd" << "I41/acd";
    g << "P3" << "P31" << "P32" << "R3" << "P-3" << "R-3" << "P312";
    g << "P321" << "P3112" << "P3121" << "P3212" << "P3221" << "R32";
    g << "P3m1" << "P31m" << "P3c1" << "P31c" << "R3m" << "R3c";
    g << "P-31m" << "P-31c" << "P-3m1" << "P-3c1" << "R-3m" << "R-3c";
    g << "P6" << "P61" << "P65" << "P62" << "P64" << "P63" << "P-6";
    g << "P6/m" << "P63/m" << "P622" << "P6122" << "P6522" << "P6222";
    g << "P6422" << "P6322" << "P6mm" << "P6cc" << "P63cm" << "P63mc";
    g << "P-6m2" << "P-6c2" << "P-62m" << "P-62c" << "P6/mmm" << "P6/mcc";
    g << "P63/mcm" << "P63/mmc" << "P23" << "F23" << "I23" << "P213";
    g << "I213" << "Pm-3" << "Pn-3" << "Fm-3" << "Fd-3" << "Im-3";
    g << "Pa-3" << "Ia-3" << "P432" << "P4232" << "F432" << "F4132" << "I432";
    g << "P4332" << "P4132" << "I4132" << "P-43m" << "F-43m" << "I-43m" << "P-43n";
    g << "F-43c" << "I-43d" << "Pm-3m" << "Pn-3n" << "Pm-3n" << "Pn-3m";
    g << "Fm-3m" << "Fm-3c" << "Fd-3m" << "Fd-3c" << "Im-3m" << "Ia-3d";

    for (int i=0; i<g.size(); i++) {
        int matched = 0;
        for (int j=0; j<groups.size(); j++) {
            if (groups[j].match(g[i])) {
                matched++;
                QStringList e = groups[j].elements();
                qDebug() << i+1 << " " << g[i] << " " << j << " " << e;
            }
        }
        if (matched!=1) {
            QString s = g[i];
            qDebug() << "Strange" << "";
        }
    }
}
    */

/*


class Spacegroup:

    triclinicPoint={'1': '(1)',
                    '-1': '(-1)'}

    monoclinicPoint={'2': '(21?)',
                     'm': '([mabc])',
                     '2/m': '(21?)/([mnabcd])'}

    orthorhombicPoint={'222': '(21?)(21?)(21?)',
                      'mm2':r'''(?=
                                   [mnbcd][mnacd]21?
                                   |
                                   [mnbcd]21?[mnabd]
                                   |
                                   21?[mnacd][mnabd])
                                   ([mnabcd]|21?)([mnabcd]|21?)([mnabcd]|21?)''',
                      'mmm': '([mnbcd])([mnacd])([mnabd])'}
    tetragonalPoint={'4': '(4[123]?)',
                     '-4': '(-4)',
                     '4/m':'(4[123]?/[mnab])',
                     '422':'(4[123]?)(21?)(21?)',
                     '4mm':'(-?4[123]?)([mnabcd])([mnabcd])',
                     '-42m':'(-4)(?=21?[mnabcd]|[mnabcd]21?)(21?|[mnabcd])(21?|[mnabcd])',
                     '4/mmm':'(4[123]?/[mnab])([mnabcd])([mnabcd])'}

    trigonalPoint={'3': '(3[12]?)',
                   '-3':'(-3)',
                   '32':'(3[12]?)(?=21|12)(.)(.)',
                   '3m':'(-?3)(?=[mnabcd]1?|1[mnabcd])(.)(.?)'}
    hexagonalPoint={'6':'(6[12345]?)',
                    '-6':'(-6)',
                    '6/m':'(63?/m)',
                    '622':'(6[12345]?)(2)(2)',
                    '6mm':'(63?)([mc])([mc])',
                    '-6m2':'(-6)(?=2[mc]|[mc]2)(.)(.)',
                    '6/mmm':'(63?/[mc])([mc])([mc])'}



    cubicPoint={'23':'(21?)(-?3)',
                'm3':'([mnabcd])(-?3)',
                '432':'(4[123]?)(-?3)(2)',
                '43m': '(-?4[123]?)(-?3)([mnabcd])',
                'm3m': '([mnabcd])(-?3)([mnabcd])'}

    pointGroups={ '1'     : triclinic,
                  '-1'    : triclinic,
                  '2'     : monoclinic,
                  'm'     : monoclinic,
                  '2/m'   : monoclinic,
                  '222'   : orthorhombic,
                  'mm2'   : orthorhombic,
                  'mmm'   : orthorhombic,
                  '3'     : trigonal,
                  '-3'    : trigonal,
                  '32'    : trigonal,
                  '3m'    : trigonal,
                  '-32/m' : trigonal,
                  '6'     : hexagonal,
                  '-6'    : hexagonal,
                  '6/m'   : hexagonal,
                  '622'   : hexagonal,
                  '6mm'   : hexagonal,
                  '-6m2'  : hexagonal,
                  '6/mmm' : hexagonal,
                  '4'     : tetragonal,
                  '-4'    : tetragonal,
                  '4/m'   : tetragonal,
                  '422'   : tetragonal,
                  '4mm'   : tetragonal,
                  '-42m'  : tetragonal,
                  '4/mmm' : tetragonal,
                  '23'    : cubic,
                  'm3'    : cubic,
                  '432'   : cubic,
                  '43m'   : cubic,
                  'm3m'   : cubic }


    inv=Mat3D((-1, 0, 0, 0, -1, 0, 0, 0, -1))
    mx =Mat3D((-1, 0, 0, 0,  1, 0, 0, 0,  1))
    my =Mat3D(( 1, 0, 0, 0, -1, 0, 0, 0,  1))
    mz =Mat3D(( 1, 0, 0, 0,  1, 0, 0, 0, -1))
    r2x=Mat3D(( 1, 0, 0, 0, -1, 0, 0, 0, -1))
    r2y=Mat3D((-1, 0, 0, 0,  1, 0, 0, 0, -1))
    r2z=Mat3D((-1, 0, 0, 0, -1, 0, 0, 0,  1))
    r4z=Mat3D(Vec3D(0, 0, 1), math.pi/2)
    r6z=Mat3D(Vec3D(0, 0, 1), math.pi/3)
    r3cub=Mat3D(Vec3D(1, 1, 1).normalized(), 2.0*math.pi/3)
    pointGroupGenerators=  {  '1'     : [],
                              '-1'    : [inv],
                              '2'     : [r2y],
                              'm'     : [my],
                              '2/m'   : [my, r2y],
                              '222'   : [r2x, r2y, r2z],
                              'mm2'   : [mx, my, r2z],
                              'mmm'   : [mx, my, mz],
                              '3'     : [],
                              '-3'    : [],
                              '32'    : [],
                              '3m'    : [],
                              '-32/m' : [],
                              '6'     : [r6z],
                              '-6'    : [r6z, inv],
                              '6/m'   : [r6z, mz],
                              '622'   : [r6z, mz],
                              '6mm'   : [r6z, mz],
                              '-6m2'  : [r6z, mz],
                              '6/mmm' : [r6z, mz],
                              '4'     : [r4z],
                              '-4'    : [r4z, inv],
                              '4/m'   : [r4z, mz],
                              '422'   : [r4z, r2x],
                              '4mm'   : [r4z, mx],
                              '-42m'  : [r4z, inv, r2x],
                              '4/mmm' : [r4z, mx, mz],
                              '23'    : [r2x, r3cub],
                              'm3'    : [mx, r3cub],
                              '432'   : [r4z, r3cub],
                              '43m'   : [r4z, r3cub],
                              'm3m'   : [inv, r4z, mx, r3cub]}

    allPointGrps=[cubicPoint, hexagonalPoint, trigonalPoint, tetragonalPoint, orthorhombicPoint, monoclinicPoint, triclinicPoint]

    centerSymbols = ['P', 'A', 'B', 'C', 'I', 'F', 'R', 'H']


    def __init__(self):
        self.symbol=None
        pass

    def parseGroupSymbol(self, s):
        pointGrp=None
        cSym=None

        self.pointGrp=None
        self.system=None
        self.symbol=s
        self.centering=None


        s=str(s)
        s=s.replace(' ','').capitalize()

        if len(s)<1:
            return False

        pGrp=s[1:]

        for grps in self.allPointGrps:
          for k in grps:
            if re.match('^'+grps[k]+'$', pGrp, re.VERBOSE):
              pointGrp=k
              break
        if (s[0] in self.centerSymbols) and pointGrp:
          self.system=self.pointGroups[pointGrp]
          if self.system!=self.trigonal and s[0] in ('H','R'):
            return False

          self.pointGrp=pointGrp
          self.symbol=s
          self.centering=s[0]
          return True
        return False

    def contrainCellToSymmetry(self, cell):
        constrain=self.getCellConstrain()
        for i in range(6):
            if constrain[i]<0:
                cell[i]=cell[-constrain[i]-1]
            elif constrain[i]>0:
                cell[i]=constrain[i]
        return cell

    def getCellConstrain(self):
        if self.system==self.triclinic:
          symConst=[0,0,0,0,0,0]
        elif self.system==self.monoclinic:
          symConst=[0,0,0,90,0,90]
        elif self.system==self.orthorhombic:
          symConst=[0,0,0,90,90,90]
        elif self.system==self.tetragonal:
          symConst=[0,-1,0,90,90,90]
        elif self.system==self.trigonal:
          if self.centering=='R':
            symConst=[0,-1,-1,0,-4,-4]
          else:
            symConst=[0,-1,0,90,90,120]
        elif self.system==self.hexagonal:
            symConst=[0,-1,0,90,90,120]
        elif self.system==self.cubic:
          symConst=[0,-1,-1,90,90,90]
        else:
          symConst=[0,0,0,0,0,0]
        return symConst

    def addGenerator(self, Set, m):
        run=True
        notChecked=Set
        while run:
            new=[]
            for e in notChecked:
                c=e*m
                for t in Set:
                    if (c-t).sqSum()<1e-5:
                        break
                else:
                    new.append(c)
            notChecked=new
            Set+=new
            run=len(new)>0
        return Set

    def getPointGroup(self):
        G=[Mat3D()]
        if self.pointGrp in self.pointGroupGenerators:
            for g in self.pointGroupGenerators[self.pointGrp]:
                self.addGenerator(G, g)
        return G

    def getLaueGroup(self):
        G=self.getPointGroup()
        self.addGenerator(G, self.inv)
        return G



        */
