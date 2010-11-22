#ifndef __CRYSTAL_H__
#define __CRYSTAL_H__

#include <QObject>
#include <QMetaType>
#include <tools/vec3D.h>
#include <tools/mat3D.h>
#include <tools/objectstore.h>
#include <core/fitobject.h>
#include "core/spacegroup.h"
#include "core/reflection.h"
#include <QFuture>
#include <QFutureWatcher>
#include <QMutex>

class Projector;


class Crystal: public QObject, public FitObject {
  Q_OBJECT
public:
  enum RotationAxisType {
    LabSystem,
    ReziprocalSpace,
    DirectSpace
  };

  Crystal(QObject* parent);
  Crystal(const Crystal &);
  ~Crystal();

  void updateRotation();

  int reflectionCount();
  Reflection getReflection(int i);
  Reflection getClosestReflection(const Vec3D& normal);
  QVector<Reflection> getReflectionList();

  Vec3D uvw2Real(const Vec3D&);
  Vec3D uvw2Real(int u, int v, int w);
  Vec3D hkl2Reziprocal(const Vec3D&);
  Vec3D hkl2Reziprocal(int h, int k, int l);

  Mat3D getRealOrientationMatrix() const;
  Mat3D getReziprocalOrientationMatrix() const;
  Mat3D getRotationMatrix() const;

  void calcEulerAngles(double &omega, double &chi, double &phi);
  void setEulerAngles(double omega, double chi, double phi);

  Spacegroup* getSpacegroup();
  bool setSpacegroup();

  Vec3D getRotationAxis() const;
  Vec3D getLabSystemRotationAxis() const;
  RotationAxisType getRotationAxisType() const;

  QList<Projector*> getConnectedProjectors();

  QList<double> getCell();
  void enableUpdate(bool b=true);

  // Functions for fitting parameters
  virtual double fitParameterValue(int n);
  virtual void fitParameterSetValue(int n, double val);
  virtual void fitParameterSetEnabled(int n, bool enable);

public slots:
  void addRotation(const Vec3D& axis, double angle);
  void addRotation(const Mat3D& M);
  void setRotation(const Mat3D& M);
  void setCell(double a, double b, double c, double alpha, double beta, double gamma);
  void setCell(QList<double>);
  void setWavevectors(double Qmin, double Qmax);
  void addProjector(Projector*);
  void removeProjector(Projector*);
  void updateWavevectorsFromProjectors();
  void setRotationAxis(const Vec3D& axis, RotationAxisType type=LabSystem);
  void slotSetSGConstrains();
  void generateReflections();

private slots:
  void convertHtoR();
  void convertRtoH();
  void setReflections(QVector<Reflection>, double);

signals:
  void cellChanged();
  void orientationChanged();
  void reflectionsUpdate();
  void rotationAxisChanged();
  void info(QString, int);
private:
  // Do the real work. Does not use possible uninitial values of objects variables
  void internalSetCell(double a, double b, double c, double alpha, double beta, double gamma);


  // Real and reziprocal orientation Matrix
  Mat3D MReal;
  Mat3D MReziprocal;

  // Rotation Matrix
  Mat3D MRot;

  // Reziprocal lattice vectors
  Vec3D astar;
  Vec3D bstar;
  Vec3D cstar;

  // Lattice constants
  double a,b,c,alpha,beta,gamma;

  // Minimal and maximal Q-Vectors
  double Qmin;
  double Qmax;

  ObjectStore connectedProjectors;

  Vec3D rotationAxis;
  RotationAxisType axisType;

  // Spacegroup, handles sys absents
  Spacegroup spaceGroup;

  // List of Reflections
  QVector<Reflection> reflections;
  // Flag, that indicates an running update of the reflection list.
  bool reflectionUpdateRunning;
  // flag to restart immediately
  bool restartReflectionUpdate;
  // Factor for ab initio prediction of the number of reflections.
  double predictionFactor;

  QFutureWatcher<void> updateWatcher;

  bool updateEnabled;

  // Function Object that perfoms the
  // update of Reflection Parameters depending on rotation
  class UpdateRef {
  public:
    UpdateRef(Crystal* _c);
    void operator()(Reflection&);
  private:
    Mat3D MRot;
    double Qmin;
    double Qmax;
  };

  // Runnable, that performes the
  // generation of reflections
  class GenerateReflection: public QRunnable {
  public:
    GenerateReflection(Crystal*);
    ~GenerateReflection();
    void run();
  private:
    QVector<Reflection> reflections;
    Crystal* c;
    int hMax;
    QAtomicInt aktualH;
    int reflectionNumber;
    QMutex mutex;
    Vec3D astar;
    Vec3D bstar;
    Vec3D cstar;
    Mat3D MReziprocal;
    double Qmax;
    double prediction;
    Spacegroup sg;
  };


};

//struct CrystalPointer {
//  Crystal* data;
//};


Q_DECLARE_METATYPE(Crystal*)




#endif
