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

#ifndef __STEREOPROJECTOR_H__
#define __STEREOPROJECTOR_H__

#include <core/projector.h>

class StereoProjector: public Projector {
  Q_OBJECT
public:
  StereoProjector(QObject* _parent=nullptr);
  static Projector* getInstance();

  virtual QPointF scattered2det(const Vec3D&) const;
  virtual QPointF scattered2det(const Vec3D&, bool& b) const;

  virtual Vec3D det2scattered(const QPointF&) const;
  virtual Vec3D det2scattered(const QPointF&, bool& b) const;

  virtual QPointF normal2det(const Vec3D&) const;
  virtual QPointF normal2det(const Vec3D&, bool& b) const;

  virtual Vec3D det2normal(const QPointF&) const;
  virtual Vec3D det2normal(const QPointF&, bool& b) const;

  virtual QString projectorName() const;
  virtual QString displayName();
  virtual QSize projectorSizeHint() const;
  //virtual double TTmax() const;

  virtual QDomElement saveToXML(QDomElement base);
  bool displaysNonscatteringReflections();
public slots:
  virtual QWidget* configWidget();
  virtual void decorateScene();
  void setDetOrientation(const Mat3D& M);
  Mat3D getDetOrientation();
  void setDisplayNonscatteringReflections(bool);
  void saveParametersAsDefault();
protected:
  virtual bool project(const Reflection &r, QPointF &);
  virtual QPair<double, double> validOrderRange(double Q, double Qscatter);
  virtual bool parseXMLElement(QDomElement);

  Mat3D localCoordinates;
  bool displayNonscatteringReflections;
};

#endif
