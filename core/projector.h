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

#ifndef PROJECTOR_H
#define PROJECTOR_H

#include <QObject>
#include <QVector>
#include <QPointF>
#include <QPointer>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsRectItem>
#include <QString>
#include <QWaitCondition>
#include <QSemaphore>
#include <QWidget>
#include <QSignalMapper>
#include <QRunnable>
#include <QMutex>
#include <QDomElement>

#include "refinement/fitobject.h"
#include "tools/vec3D.h"
#include "tools/mat3D.h"
#include "tools/itemstore.h"

class Crystal;
class Reflection;
class RulerItem;
class SpotItem;
class ZoneItem;
class AbstractMarkerItem;
class CropMarker;
class LaueImage;
class SpotIndicatorGraphicsItem;


class Projector: public FitObject {
  Q_OBJECT
public:
  Projector(QObject* _parent=nullptr);
  virtual ~Projector();

  virtual Projector& operator=(const Projector& o);

  static const char Settings_QRangeMin[];
  static const char Settings_QRangeMax[];
  static const char Settings_maxHKLSqSum[];
  static const char Settings_textSizeFraction[];
  static const char Settings_spotSizeFraction[];

  // Functions for transformations in the different Coordinate systems

  static Vec3D normal2scattered(const Vec3D&);
  static Vec3D normal2scattered(const Vec3D&, bool &b);
  static Vec3D scattered2normal(const Vec3D&);
  static Vec3D scattered2normal(const Vec3D&, bool &b);

  QTransform det2img;
  QTransform img2det;

  virtual QPointF scattered2det(const Vec3D&) const = 0;
  virtual QPointF scattered2det(const Vec3D&, bool& b) const = 0;

  virtual Vec3D det2scattered(const QPointF&) const =0;
  virtual Vec3D det2scattered(const QPointF&, bool& b) const = 0;

  virtual QPointF normal2det(const Vec3D&) const = 0;
  virtual QPointF normal2det(const Vec3D&, bool& b) const = 0;

  virtual Vec3D det2normal(const QPointF&) const = 0;
  virtual Vec3D det2normal(const QPointF&, bool& b) const = 0;

  Reflection getClosestReflection(const Vec3D& normal);
  QList<Reflection> getProjectedReflections();
  QList<Reflection> getProjectedReflectionsNormalToZone(const TVec3D<int>& uvw);

  QGraphicsScene* getScene();
  Crystal* getCrystal();
  virtual QWidget* configWidget()=0;
  LaueImage* getLaueImage();

  virtual QString projectorName() const = 0;
  virtual QString displayName()=0;
  virtual QSize projectorSizeHint() const = 0;
  virtual QString FitObjectName();
  virtual QString fillInfoTable(const QString&);

  double Qmin() const;
  double Qmax() const;

  virtual double TTmin() const;
  virtual double TTmax() const;

  int getMaxHklSqSum() const;
  double getTextSize() const;
  double getTextSizeFraction() const;
  double getSpotSize() const;
  double getSpotSizeFraction() const;
  bool spotsEnabled() const;
  bool markersEnabled() const;
  bool isProjectionEnabled() const;

  virtual QDomElement saveToXML(QDomElement base);
  bool loadFromXML(QDomElement base);

  ItemStore<SpotItem>& spotMarkers();
  void addSpotMarker(const QPointF&);
  QPointF getSpotMarkerDetPos(int n);
  QList<Vec3D> getSpotMarkerNormals();

  ItemStore<ZoneItem>& zoneMarkers();
  void addZoneMarker(const QPointF& p1, const QPointF& p2);
  QList<Vec3D> getZoneMarkerNormals();

  bool hasMarkers();
  QList<AbstractMarkerItem*> getAllMarkers();

  ItemStore<RulerItem>& rulers();
  void addRuler(const QPointF& p1, const QPointF& p2);
  QPair<QPointF, QPointF> getRulerCoordinates(int);

  ItemStore<QGraphicsRectItem>& infoItems();
  void addInfoItem(const QString& text, const QPointF& p);

  void showCropMarker();
  CropMarker* getCropMarker();

public slots:
  void delCropMarker();
  void setCrop(QPolygonF);

  void deleteMarker(AbstractMarkerItem*);

  void connectToCrystal(Crystal *);

  // Set Wavevectors. Note that the Value is expressed in 2*pi/lambda (1/A)
  void setWavevectors(double Qmin, double Qmax);
  void doProjection();

  void addRotation(const Vec3D &axis, double angle);
  void addRotation(const Mat3D& M);
  void setRotation(const Mat3D& M);

  void setSpotHighlighting(Vec3D hkl);

  virtual void decorateScene()=0;
  void setMaxHklSqSum(int m);
  void setTextSizeFraction(double d);
  void setSpotSizeFraction(double d);

  void enableSpots(bool b=true);
  void enableMarkers(bool b=true);
  void setHQPrintMode(bool b=false);

  // For speedup of fitting...
  void enableProjection(bool b=true);

  void loadImage(QString);
  void closeImage();

  virtual void doImgRotation(const QTransform&);
  virtual void saveParametersAsDefault();
signals:
  void projectedPointsUpdated();
  void wavevectorsUpdated();
  void projectionParamsChanged();
  void projectionRectPosChanged();
  void projectionRectSizeChanged();
  void imgTransformUpdated();
  void imageLoaded(LaueImage*);
  void imageClosed();
  void spotSizeChanged(double);
  void textSizeChanged(double);
  void markerAdded(AbstractMarkerItem*);
  void markerChanged(AbstractMarkerItem*);
  void markerClicked(AbstractMarkerItem*);
  void markerRemoved(AbstractMarkerItem*);
  void spotHighlightChanged(Vec3D, QString);
  void projectorSavesDefault();
protected slots:
  virtual void updateImgTransformations();
  void invalidateMarkerCache();
  void setImage(LaueImage*);

  void spotMarkerAdded(int);
  void spotMarkerChanged(int);
  void spotMarkerClicked(int);
  void spotMarkerRemoved(int);
  void zoneMarkerAdded(int);
  void zoneMarkerChanged(int);
  void zoneMarkerClicked(int);
  void zoneMarkerRemoved(int);

protected:
  void internalSetWavevectors(double, double);
  void updateSpotHighlightMarker();
  virtual bool project(const Reflection &r, QPointF &point)=0;
  virtual QString diffractionOrders(const Vec3D& hkl);
  virtual QPair<double, double> validOrderRange(double Q, double Qscatter)=0;
  virtual bool parseXMLElement(QDomElement e);

  // Stuff like Primary beam marker, Coordinate lines
  QList<QGraphicsItem*> decorationItems;
  // written indexes in the scene
  QList<QGraphicsItem*> textMarkerItems;
  // Markers for indexation and fit
  ItemStore<SpotItem> spotMarkerStore;
  // Zone markers
  ItemStore<ZoneItem> zoneMarkerStore;
  // Ruler Item
  ItemStore<RulerItem> rulerStore;
  // Info Items. These will be set on Mousepress and deleted on orientation change or slot
  ItemStore<QGraphicsRectItem> infoStore;
  // The Crop Marker
  QPointer<CropMarker> cropMarker;


  // Pointer to the connected crystal
  QPointer<Crystal> crystal;

  double QminVal;
  double QmaxVal;
  int maxHklSqSum;
  double textSizeFraction;
  double spotSizeFraction;
  bool projectionEnabled;
  bool showMarkers;
  QVector<bool> reflectionIsProjected;

  QGraphicsScene scene;

  // _parentfor Items "living" on the image, e.g. in the Rect(0,0)->(1,1)
  QGraphicsPixmapItem* imageItemsPlane;

  // Projected Spots, drawing is done multithreaded
  SpotIndicatorGraphicsItem* spotIndicator;

  LaueImage* imageData;

  Vec3D spotHighlightHKL;
  QGraphicsItem* spotHighlightItem;
};


Q_DECLARE_METATYPE(QVector<QPointF>);
Q_DECLARE_METATYPE(QList<QGraphicsItem*>);

#endif
