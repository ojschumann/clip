#ifndef __STEREOPROJECTOR_H__
#define __STEREOPROJECTOR_H__

#include <core/projector.h>

class StereoProjector: public Projector {
  Q_OBJECT
public:
  StereoProjector(QObject* parent=0);
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
public slots:
  virtual QWidget* configWidget();
  virtual void decorateScene();
  void setDetOrientation(const Mat3D& M);
  Mat3D getDetOrientation();
protected:
  virtual bool project(const Reflection &r, QPointF &);
  virtual bool parseXMLElement(QDomElement);

  Mat3D localCoordinates;
};

#endif
