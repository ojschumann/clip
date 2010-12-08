#ifndef SPACEGROUP_H
#define SPACEGROUP_H

#include <QObject>
#include <QString>
#include <QList>
#include <QStringList>
#include <QFlags>
#include <QRegExp>
#include <tools/vec3D.h>
#include <tools/mat3D.h>

class SpacegroupCheck;

class Spacegroup: public QObject
{
  Q_OBJECT
public:
  enum Systems {
    triclinic,
    monoclinic,
    orthorhombic,
    trigonal,
    hexagonal,
    tetragonal,
    cubic
  };
  Q_DECLARE_FLAGS(System, Systems)


  Spacegroup(QObject* parent=0);
  Spacegroup(const Spacegroup&);
  bool setGroupSymbol(QString);

  QString groupSymbol() const;
  System crystalSystem() const;
  QList<int> getConstrains() const;

  bool isExtinct(const TVec3D<int>&);


signals:
  void groupChanged();
  void constrainsChanged();
  void triclinHtoR();
  void triclinRtoH();


private:

  class SpacegroupSymbolInfo {
  public:
    SpacegroupSymbolInfo(int, QString, QString, QString, QString);
    bool match(QString);

    Spacegroup::System system() const;
    int SpacegroupNumber() const;
    QString HerrmannMauguin() const;
    QString Hall() const;
  private:
    // Number of Spacegroup in ITs
    int spacegroupNumber;
    // Gives additional info on Setting for Number in International Tables
    QString spacegroupNumberModifier;
    // Hermann-Mauguin Symbol with spaces
    QString hermannMauguinSymbol;
    // Hermann-Mauguin Symbol without spaces
    QString compactHermannMauguinSymbol;
    // Gives additional info on Setting for Number in International Tables
    QString hermannMauguinModifier;
    // Hall Symbol
    QString hallSymbol;
  };
  static QList<SpacegroupSymbolInfo> static_init();


  QString symbol;
  System crystalsystem;

  static QList<SpacegroupSymbolInfo> groupInfos;

  class GroupElement {
  public:
    GroupElement(int m11, int m12, int m13, int m21, int m22, int m23, int m31, int m32, int m33, int t1, int t2, int t3);
    GroupElement(const TMat3D<int>&, const TVec3D<int>&);
    void normalize();
    void print() const;
    GroupElement operator*(const GroupElement&) const;
    bool operator==(const GroupElement&) const;
    TMat3D<int> M;
    TVec3D<int> t;
    static const int MOD = 12;
  };

  struct ExtinctionElement {
    TMat3D<int> M;
    TVec3D<int> t;
  };

  QList<GroupElement> group;
  QList<TMat3D<int> > pointgroup;
  QList<ExtinctionElement> extinctionChecks;
  QList<int> constrains;

  bool generateGroup(QString);
  template <class T> void addToGroup(QList<T>& group, const T&);

  /*struct PGElem{
        Mat3D R;
        Vec3D t;
    };

    QList<PGElem> pointgroup;
*/
};






#endif // SPACEGROUP_H
