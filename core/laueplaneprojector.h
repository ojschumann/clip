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

#ifndef LAUEPLANEPROJECTOR_H
#define LAUEPLANEPROJECTOR_H

#include "core/projector.h"
#include "refinement/fitparametergroup.h"

#include "image/imagedatastore.h"

class LauePlaneProjector: public Projector {
  Q_OBJECT
public:
  LauePlaneProjector(QObject* _parent=nullptr);

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
  virtual QString fillInfoTable(const QString &);
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
  void movedPrimaryBeamMarker();
  void updatePrimaryBeamPos();
  virtual void loadParmetersFromImage(LaueImage*);
  virtual void saveParametersAsDefault();
  void loadNewPhysicalImageSize(ImageDataStore::DataType=ImageDataStore::PhysicalSize);
protected:
  double maxCos(Vec3D n) const;
  virtual bool parseXMLElement(QDomElement e);

  virtual bool project(const Reflection &r, QPointF& p);
  virtual QPair<double, double> validOrderRange(double Q, double Qscatter);

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
  LauePlaneProjector(const LauePlaneProjector&);
  LauePlaneProjector& operator=(const LauePlaneProjector&);
};

#endif
