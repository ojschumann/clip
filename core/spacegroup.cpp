#include "spacegroup.h"

#include <QRegExp>
#include <QStringList>
#include <QDebug>


SpaceGroup::SpaceGroupCheck::SpaceGroupCheck(SpaceGroup::System s, QString pg, QString re) {
  system = s;
  pointgroup = pg;
  regexp = QRegExp(re);
}

bool SpaceGroup::SpaceGroupCheck::match(QString s) {
  return regexp.exactMatch(s);
}

QStringList SpaceGroup::SpaceGroupCheck::elements() {
  QStringList r = regexp.capturedTexts();
  if (not r.empty())
    r.takeFirst();
  return r;
}



SpaceGroup::SpaceGroup(QObject* parent=NULL): QObject(parent), groups() {
  groups << SpaceGroupCheck(   triclinic,    "1", "([PABCIF])(1)");
  groups << SpaceGroupCheck(   triclinic,   "-1", "([PABCIF])(-1)");

  groups << SpaceGroupCheck(  monoclinic,    "2", "([PABCIF])(21?)");
  groups << SpaceGroupCheck(  monoclinic,    "m", "([PABCIF])([mabc])");
  groups << SpaceGroupCheck(  monoclinic,  "2/m", "([PABCIF])(21?/[mnabcd])");

  groups << SpaceGroupCheck(orthorhombic,  "222", "([PABCIF])(21?)(21?)(21?)");
  groups << SpaceGroupCheck(orthorhombic,  "mm2", "([PABCIF])(?=[mnbcde][mnacde]21?|[mnbcde]21?[mnabde]|21?[mnacde][mnabde])([mnabcde]|21?)([mnabcde]|21?)([mnabcde]|21?)");
  groups << SpaceGroupCheck(orthorhombic,  "mmm", "([PABCIF])([mnbcde])([mnacde])([mnabde])");

  groups << SpaceGroupCheck(  tetragonal,    "4", "([PCIF])(4[123]?)");
  groups << SpaceGroupCheck(  tetragonal,   "-4", "([PCIF])(-4[123]?)");
  groups << SpaceGroupCheck(  tetragonal,  "4/m", "([PCIF])(4[123]?/[mnab])");
  groups << SpaceGroupCheck(  tetragonal,  "422", "([PCIF])(4[123]?)(21?)(21?)");
  groups << SpaceGroupCheck(  tetragonal,  "4mm", "([PCIF])(4[123]?)([mnabcd])([mnabcd])");
  groups << SpaceGroupCheck(  tetragonal, "-42m", "([PCIF])(-4[123]?)(?=21?[mnabcd]|[mnabcd]21?)(21?|[mnabcd])(21?|[mnabcd])");
  groups << SpaceGroupCheck(  tetragonal,"4/mmm", "([PCIF])(4[123]?/[mnab])([mnabcd])([mnabcd])");

  groups << SpaceGroupCheck(    trigonal,    "3", "([PRH])(3[12]?)");
  groups << SpaceGroupCheck(    trigonal,   "-3", "([PRH])(-3)");
  groups << SpaceGroupCheck(    trigonal,   "32", "([PRH])(-?3[12]?)(?=21|12)(.)(.)");
  groups << SpaceGroupCheck(    trigonal,   "3m", "([PRH])(-?3)(?=[mnabcd]1?|1[mnabcd])(.)(.?)");

  groups << SpaceGroupCheck(   hexagonal,    "6", "([PH])(6[12345]?)");
  groups << SpaceGroupCheck(   hexagonal,   "-6", "([PH])(-6)");
  groups << SpaceGroupCheck(   hexagonal,  "6/m", "([PH])(63?/m)");
  groups << SpaceGroupCheck(   hexagonal,  "622", "([PH])(6[12345]?)(2)(2)");
  groups << SpaceGroupCheck(   hexagonal,  "6mm", "([PH])(63?)([mc])([mc])");
  groups << SpaceGroupCheck(   hexagonal, "-6m2", "([PH])(-6)(?=2[mc]|[mc]2)(.)(.)");
  groups << SpaceGroupCheck(   hexagonal,"6/mmm", "([PH])(63?/[mc])([mc])([mc])");

  groups << SpaceGroupCheck(       cubic,   "23", "([PIF])(21?)(-?3)");
  groups << SpaceGroupCheck(       cubic,   "m3", "([PIF])([mnabcd])(-?3)");
  groups << SpaceGroupCheck(       cubic,  "432", "([PIF])(4[123]?)(-?3)(2)");
  groups << SpaceGroupCheck(       cubic,  "43m", "([PIF])(-?4[123]?)(-?3)([mnabcd])");
  groups << SpaceGroupCheck(       cubic,  "m3m", "([PIF])([mnabcd])(-?3)([mnabcd])");

  setGroupSymbol("P1");
}

bool SpaceGroup::setGroupSymbol(QString s) {
  QList<SpaceGroupCheck>::iterator iter;
  for (iter=groups.begin(); iter!=groups.end(); ++iter) {
    if (iter->match(s)) {

      bool SystemChg = crystalsystem!=iter->system;
      bool startup = false;
      QString CenterChg;
      if (symbolElements.empty()) {
        startup = true;
      } else {
        CenterChg = symbolElements.first();
      }

      symbol = s;
      crystalsystem = iter->system;
      symbolElements = iter->elements();
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
      generatePointgroup();
      emit groupChanged();
      return true;
    }
  }
  return false;
}

QString SpaceGroup::groupSymbol() const {
  return symbol;
}

SpaceGroup::System SpaceGroup::crystalSystem() const {
  return crystalsystem;
}

QList<int> SpaceGroup::getConstrains() const {
  QList<int> r;
  if (crystalsystem==triclinic) {
    r << 0 << 0 << 0 << 0 << 0 << 0;
  } else if (crystalsystem==monoclinic) {
    r << 0 << 0 << 0 << -5 << 0 << -5;
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


SpaceGroup::PointgroupElement::PointgroupElement(int m11, int m12, int m13, int m21, int m22, int m23, int m31, int m32, int m33, int t1, int t2, int t3): M(m11, m12, m13, m21, m22, m23, m31, m32, m33), t(t1, t2, t3) {
  normalize();
};

SpaceGroup::PointgroupElement::PointgroupElement(const TMat3D<int> &_M, const TVec3D<int> &_t): M(_M), t(_t) {
  normalize();
};

void SpaceGroup::PointgroupElement::normalize() {
  for (int i=0; i<3; i++) {
    t(i)%=6;
    if (t(i)<0)
      t(i)+=6;
  }
}

SpaceGroup::PointgroupElement SpaceGroup::PointgroupElement::operator *(const SpaceGroup::PointgroupElement& o) {
  return SpaceGroup::PointgroupElement(M*o.M, M*o.t + t);
}

bool SpaceGroup::PointgroupElement::operator==(const SpaceGroup::PointgroupElement& o) {
  return (M==o.M) && t==o.t;
}


bool SpaceGroup::isExtinct(const TVec3D<int>& reflection) {
  for (int i=0; i<extinctionChecks.size(); i++) {
    int s = reflection*extinctionChecks.at(i).t;
    if ((s%6)!=0) {
      if ((extinctionChecks.at(i).M*reflection).isNull()) {
        return true;
      }
    }
  }
  return false;
}

void SpaceGroup::generatePointgroup() {
  pointgroup.clear();
  extinctionChecks.clear();

  pointgroup << PointgroupElement(1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0);
  if (symbolElements.first()=="I") {
    pointgroup << PointgroupElement(1, 0, 0, 0, 1, 0, 0, 0, 1, 3, 3, 3);
  } else if (symbolElements.first()=="F") {
    pointgroup << PointgroupElement(1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 3, 3);
    pointgroup << PointgroupElement(1, 0, 0, 0, 1, 0, 0, 0, 1, 3, 0, 3);
    pointgroup << PointgroupElement(1, 0, 0, 0, 1, 0, 0, 0, 1, 3, 3, 0);
  } else if (symbolElements.first()=="A") {
    pointgroup << PointgroupElement(1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 3, 3);
  } else if (symbolElements.first()=="B") {
    pointgroup << PointgroupElement(1, 0, 0, 0, 1, 0, 0, 0, 1, 3, 0, 3);
  } else if (symbolElements.first()=="C") {
    pointgroup << PointgroupElement(1, 0, 0, 0, 1, 0, 0, 0, 1, 3, 3, 0);
  } else if (symbolElements.first()=="R") {
  } else if (symbolElements.first()=="H") {
    pointgroup << PointgroupElement(1, 0, 0, 0, 1, 0, 0, 0, 1, 4, 2, 2);
    //pointgroup << PointgroupElement(1, 0, 0, 0, 1, 0, 0, 0, 1, 2, 4, 4);
    //pointgroup << PointgroupElement(1, 0, 0, 0, 1, 0, 0, 0, 1, -2, 2, 2);
    //pointgroup << PointgroupElement(1, 0, 0, 0, 1, 0, 0, 0, 1, -2, -4, 2);
    pointgroup << PointgroupElement(0,-1, 0, 1, -1, 0, 0, 0, 1, 0, 0, 0);
  }

  int lastGSize;
  do {
    lastGSize = pointgroup.size();
    for (int i=1; i<pointgroup.size(); i++) {
      for (int j=1; j<pointgroup.size(); j++) {
        PointgroupElement test = pointgroup[i]*pointgroup[j];
        bool uniq=true;
        for (int k=0; k<pointgroup.size(); k++) {
          if (test==pointgroup[k]) {
            uniq = false;
            break;
          }
        }
        if (uniq) {
          pointgroup << test;
        }

      }
    }
  } while(lastGSize!=pointgroup.size());
  for (int i=0; i<pointgroup.size(); i++) {
    if (!pointgroup.at(i).t.isNull()) {
      ExtinctionElement e;
      e.M=TMat3D<int>() - pointgroup.at(i).M;
      e.t=pointgroup.at(i).t;
      extinctionChecks << e;
    }
  }
}

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


class SpaceGroup:

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
