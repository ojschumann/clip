#ifndef __PROJECTOR_H__
#define __PROJECTOR_H__

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
#include <QThread>
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
  Projector(QObject* parent=0);
  virtual ~Projector();

  virtual Projector& operator=(const Projector& o);


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

  QGraphicsScene* getScene();
  Crystal* getCrystal();
  virtual QWidget* configWidget()=0;
  LaueImage* getLaueImage();

  virtual QString projectorName()=0;
  virtual QString displayName()=0;
  virtual QString FitObjectName();

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

  void connectToCrystal(Crystal *);

  // Set Wavevectors. Note that the Value is 1/lambda, not 2*pi/lambda
  // TODO: Still valid???
  void setWavevectors(double Qmin, double Qmax);
  void reflectionsUpdated();

  void addRotation(const Vec3D &axis, double angle);
  void addRotation(const Mat3D& M);
  void setRotation(const Mat3D& M);

  virtual void decorateScene()=0;
  void setMaxHklSqSum(int m);
  void setTextSizeFraction(double d);
  void setSpotSizeFraction(double d);
  void enableSpots(bool b=true);

  // For speedup of fitting...
  //TODO: Check if nessesary
  void enableProjection(bool b=true);

  void loadImage(QString);
  void closeImage();

  virtual void doImgRotation(const QTransform&);
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
  void markerAdded();
  void markerRemoved();
protected slots:
  virtual void updateImgTransformations();
  void invalidateMarkerCache();
  void setImage(LaueImage*);
protected:
  void internalSetWavevectors(double, double);
  virtual bool project(const Reflection &r, QPointF &point)=0;
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
  // Info Items. These will be set on Mousepress from Python and be deleted on orientation change or slot!
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
  bool showSpots;
  bool projectionEnabled;
  QVector<bool> reflectionIsProjected;

  QGraphicsScene scene;

  // Parent for Items "living" on the image, e.g. in the Rect(0,0)->(1,1)
  QGraphicsPixmapItem* imageItemsPlane;

  // Projected Spots, drawing is done multithreaded
  SpotIndicatorGraphicsItem* spotIndicator;

  LaueImage* imageData;
};


Q_DECLARE_METATYPE(QVector<QPointF>);
Q_DECLARE_METATYPE(QList<QGraphicsItem*>);

#endif
