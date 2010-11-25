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

  class SpacegroupCheck {
  public:
    SpacegroupCheck(Spacegroup::System, QString, QString);
    bool match(QString);
    QStringList elements();

    Spacegroup::System system;
    QString pointgroup;
    QRegExp regexp;
  };


  QString symbol;
  QStringList symbolElements;
  System crystalsystem;

  QList<SpacegroupCheck> groups;

  class PointgroupElement {
  public:
    PointgroupElement(int m11, int m12, int m13, int m21, int m22, int m23, int m31, int m32, int m33, int t1, int t2, int t3);
    PointgroupElement(const TMat3D<int>&, const TVec3D<int>&);
    void normalize();
    PointgroupElement operator*(const PointgroupElement&);
    bool operator==(const PointgroupElement&);
    TMat3D<int> M;
    TVec3D<int> t;
  };

  struct ExtinctionElement {
    TMat3D<int> M;
    TVec3D<int> t;
  };

  QList<PointgroupElement> pointgroup;
  QList<ExtinctionElement> extinctionChecks;

  void generatePointgroup();
  void addToPointgroup(const PointgroupElement&);

  /*struct PGElem{
        Mat3D R;
        Vec3D t;
    };

    QList<PGElem> pointgroup;
*/
};






#endif // SPACEGROUP_H
