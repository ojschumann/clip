#ifndef __LAUEPLANEPROJECTOR_H__
#define __LAUEPLANEPROJECTOR_H__

#include "core/projector.h"
#include "refinement/fitparametergroup.h"

class LauePlaneProjector: public Projector {
  Q_OBJECT
public:
  LauePlaneProjector(QObject* parent=0);

  virtual Projector& operator=(const Projector& o);

  // Needed for ProjectorFactory
  static Projector* getInstance();

  virtual QPointF scattered2det(const Vec3D&) const;
  virtual QPointF scattered2det(const Vec3D&, bool& b) const;

  virtual Vec3D det2scattered(const QPointF&) const;
  virtual Vec3D det2scattered(const QPointF&, bool& b) const;

  virtual QPointF normal2det(const Vec3D&) const;
  virtual QPointF normal2det(const Vec3D&, bool& b) const;

  virtual Vec3D det2normal(const QPointF&) const;
  virtual Vec3D det2normal(const QPointF&, bool& b) const;

  virtual QWidget* configWidget();
  virtual QString projectorName() const;
  QSize projectorSizeHint() const;
  virtual QString displayName();
  double dist() const;
  double width() const;
  double height() const;
  double omega() const;
  double chi() const;
  double phi() const;
  double xOffset() const;
  double yOffset() const;

  virtual double TTmin() const;
  virtual double TTmax() const;

  virtual QDomElement saveToXML(QDomElement base);
public slots:
  void setDetSize(double dist, double width, double height);
  void setDetOrientation(double omega, double chi, double phi);
  void setDetOffset(double dx, double dy);
  void setDist(double);
  void setWidth(double);
  void setHeight(double);
  void setOmega(double);
  void setChi(double);
  void setPhi(double);
  void setXOffset(double);
  void setYOffset(double);
  virtual void decorateScene();
  void resizePBMarker();
  void movedPBMarker();
  void updatePBPos();
  virtual void doImgRotation(const QTransform& t);
  virtual void loadParmetersFromImage(LaueImage*);
  virtual void saveParametersAsDefault();
protected:
  double maxCos(Vec3D n) const;
  virtual bool parseXMLElement(QDomElement e);

  virtual bool project(const Reflection &r, QPointF& p);

  Mat3D localCoordinates;
  double detDist;
  double detWidth;
  double detHeight;
  double detOmega;
  double detChi;
  double detPhi;
  double detDx;
  double detDy;

  // FitParameterGroups for fitting
  class DistGroup: public FitParameterGroup {
  public:
    DistGroup(LauePlaneProjector* p);
    virtual double value(int member) const;
    virtual double epsilon(int member) const;
    virtual double lowerBound(int member) const;
    virtual double upperBound(int member) const;
  protected:
    virtual void doSetValue(QList<double> values);
    LauePlaneProjector* projector;
  };

  class OrientationGroup: public FitParameterGroup {
  public:
    OrientationGroup(LauePlaneProjector* p);
    virtual double value(int member) const;
    virtual double epsilon(int member) const;
    virtual double lowerBound(int member) const;
    virtual double upperBound(int member) const;
  protected:
    virtual void doSetValue(QList<double> values);
    LauePlaneProjector* projector;
  };

  class ShiftGroup: public FitParameterGroup {
  public:
    ShiftGroup(LauePlaneProjector* p);
    virtual double value(int member) const;
    virtual double epsilon(int member) const;
    virtual double lowerBound(int member) const;
    virtual double upperBound(int member) const;
  protected:
    virtual void doSetValue(QList<double> values);
    LauePlaneProjector* projector;
  };

  DistGroup distGroup;
  OrientationGroup orientationGroup;
  ShiftGroup shiftGroup;

private:
  LauePlaneProjector(const LauePlaneProjector&):
      distGroup(this),
      orientationGroup(this),
      shiftGroup(this) {};
};

#endif
