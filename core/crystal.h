/**********************************************************************
  Copyright (C) 2008-2011 Olaf J. Schumann

  This file is part of the Cologne Laue Indexation Program.
  For more information, see <http://clip4.sf.net>

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

#ifndef CRYSTAL_H
#define CRYSTAL_H

#include <QObject>
#include <QMetaType>
#include <QFutureWatcher>
#include <QDomElement>
#include <QXmlStreamWriter>
#include <QTime>

#include "tools/tools.h"
#include "tools/vec3D.h"
#include "tools/mat3D.h"
#include "tools/objectstore.h"
#include "refinement/fitobject.h"
#include "refinement/fitparametergroup.h"
#include "core/spacegroup.h"
#include "core/reflection.h"

class Projector;
class AbstractMarkerItem;
class ThreadRunner;

class Crystal: public FitObject {
  Q_OBJECT
public:
  enum RotationAxisType {
    LabSystem,
    ReziprocalSpace,
    DirectSpace
  };

  static const char Settings_Group[];
  static const char Settings_Spacegroup[];
  static const char Settings_CellA[];
  static const char Settings_CellB[];
  static const char Settings_CellC[];
  static const char Settings_CellAlpha[];
  static const char Settings_CellBeta[];
  static const char Settings_CellGamma[];
  static const char Settings_EulerOmega[];
  static const char Settings_EulerChi[];
  static const char Settings_EulerPhi[];

  Crystal(QObject* _parent=nullptr);
  virtual ~Crystal();

  virtual Crystal& operator=(const Crystal& o);

  void saveToXML(QDomElement base);
  bool loadFromXML(QDomElement base);

  virtual QString FitObjectName();
  virtual void prepareForFit();

  int reflectionCount();
  Reflection getReflection(int i);
  Reflection makeReflection(const TVec3D<int>& hkl) const;
  Reflection getClosestReflection(const Vec3D& normal);
  QVector<Reflection> getReflectionList();

  Vec3D uvw2Real(const Vec3D&);
  Vec3D uvw2Real(int u, int v, int w);
  Vec3D hkl2Reziprocal(const Vec3D&);
  Vec3D hkl2Reziprocal(int h, int k, int l);

  Mat3D getRealOrientationMatrix() const;
  Mat3D getReziprocalOrientationMatrix() const;
  Mat3D getRotationMatrix() const;

  QList<double> calcEulerAngles(bool inDegrees=false);
  void setEulerAngles(double omega, double chi, double phi);

  Spacegroup* getSpacegroup();
  bool setSpacegroup();

  Vec3D getRotationAxis() const;
  Vec3D getLabSystemRotationAxis() const;
  RotationAxisType getRotationAxisType() const;

  QList<Projector*> getConnectedProjectors();
  QList<AbstractMarkerItem*> getMarkers();
  virtual QList<FitObject*> getFitObjects();

  QList<double> getCell();
  void enableUpdate(bool b=true);  
  void synchronUpdate(bool b=true);

public slots:
  void addRotation(const Vec3D& axis, double angle);
  void addRotation(const Mat3D& M);
  void setRotation(const Mat3D& M);
  void updateRotation();
  void setCell(double a, double b, double c, double alpha, double beta, double gamma);
  void setCell(QList<double>);
  void setWavevectors(double Qmin, double Qmax);
  void addProjector(Projector*);
  void removeProjector(Projector*);
  void updateWavevectorsFromProjectors();
  void setRotationAxis(const Vec3D& axis, RotationAxisType type=LabSystem);
  void slotSetSGConstrains();
  void generateReflections();
  void saveParametersAsDefault();
private slots:
  void convertHtoR();
  void convertRtoH();
  void reflectionGenerated();
signals:
  void cellChanged();
  void orientationChanged();
  void reflectionsUpdate();
  void rotationAxisChanged();
  void info(QString, int);
  void projectorAdded(Projector*);
  void projectorRemoved(Projector*);
  void markerAdded(AbstractMarkerItem*);
  void markerChanged(AbstractMarkerItem*);
  void markerClicked(AbstractMarkerItem*);
  void markerRemoved(AbstractMarkerItem*);
  void deleteMarker(AbstractMarkerItem*);
private:
  // Do the real work. Does not use possible uninitial values of objects variables
  void internalSetCell(double a, double b, double c, double alpha, double beta, double gamma);


  struct GenerationParameters {
    GenerationParameters(Crystal*);
    Mat3D MRot;
    Mat3D MReziprocal;
    Spacegroup spacegroup;
    double qMin;
    double qMax;
    double predictionFactor;
  };

  // define as static to avoid accidential use of this-ptr
  static QPair<QVector<Reflection>, double> doGeneration(const GenerationParameters&);


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
  QFutureWatcher<QPair<QVector<Reflection>, double> > reflectionFuture;
  // flag to restart generation of reflections immediately
  bool restartReflectionUpdate;
  // flag to immediately update the rotation on newly generated reflections
  // nessesary if rotation is altered during generation of reflections
  bool immediateRotationUpdate;
  // Factor for ab initio prediction of the number of reflections.
  double predictionFactor;

  bool updateEnabled;
  bool updateIsSynchron;

  // Function Object that perfoms the
  // update of Reflection Parameters depending on rotation
  class UpdateRef {
  public:
    UpdateRef(const Crystal* _c);
    UpdateRef(const Mat3D& rot, double qmin, double qmax);
    void operator()(Reflection&) const;
  private:
    Mat3D MRot;
    double Qmin;
    double Qmax;
  };

  class UpdateLoadBalancer {
  public:
    UpdateLoadBalancer(Reflection* _d, int _size, const UpdateRef &_u);
    void operator()();
  private:
    Reflection* d;
    int size;
    const UpdateRef& u;
    //std::atomic<int> wp;
    QAtomicInt wp;
    int chunkSize;
  };

  ThreadRunner* reflectionsUpdater;

  // FitParameterGroups for fitting
  class CellGroup: public FitParameterGroup {
  public:
    CellGroup(Crystal* c);
    virtual double value(int member) const;
    virtual double epsilon(int member) const;
    virtual double lowerBound(int member) const;
    virtual double upperBound(int member) const;
    virtual void notifySetEnabled(int member, bool b);
    virtual void notifySetChangeable(int member, bool b);
  protected:
    virtual void doSetValue(QList<double> values);
    Crystal* crystal;
  };

  // FitParameterGroups for fitting
  class OrientationGroup: public FitParameterGroup {
  public:
    OrientationGroup(Crystal* c);
    virtual double value(int member) const;
    virtual double epsilon(int member) const;
    virtual double lowerBound(int member) const;
    virtual double upperBound(int member) const;
    void setBaseRotation(const Mat3D& R);
  protected:
    virtual void doSetValue(QList<double> values);
    Crystal* crystal;
    Mat3D baseRotation;
    double omega, chi, phi;
  };

  CellGroup cellGroup;
  OrientationGroup orientationGroup;

#ifdef CLIP_DEBUG
public slots:
  void enableDebug(bool b);
  void debugSlot();
private:
  bool debugEnabled;
  int debugCallsInActualSecond;
  Mean debugMean;
  QTime debugTimer;
#endif
};

Q_DECLARE_METATYPE(Crystal*)




#endif
