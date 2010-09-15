#ifndef __PROJECTOR_H__
#define __PROJECTOR_H__

#include <core/reflection.h>
#include <QtCore/QObject>
#include <QtCore/QVector>
#include <QtCore/QPointF>
#include <QtCore/QPointer>
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsItem>
#include <QtCore/QString>
#include <QtCore/QXmlStreamReader>
#include <QtCore/QXmlStreamWriter>
#include <core/crystal.h>
#include <core/fitobject.h>

class Projector: public QObject, public FitObject {
    Q_OBJECT
    public:
        Projector(QObject* parent=0);
        Projector(const Projector&);
    
        // Functions for transformations in the different Coordinate systems
    
        static Vec3D normal2scattered(const Vec3D&, bool* b=NULL);
        static Vec3D scattered2normal(const Vec3D&, bool* b=NULL);
        
        QTransform det2img;
        QTransform img2det;
    
        virtual QPointF scattered2det(const Vec3D&, bool* b=NULL) const =0;
        virtual Vec3D det2scattered(const QPointF&, bool* b=NULL) const =0;
        virtual QPointF normal2det(const Vec3D&, bool* b=NULL) const =0;
        virtual Vec3D det2normal(const QPointF&, bool* b=NULL) const =0;
        
        QGraphicsScene* getScene();
        Crystal* getCrystal();
        virtual QString configName()=0;
        virtual QString projectorName()=0;
        virtual QString displayName()=0;
        
        double Qmin() const;
        double Qmax() const;

        virtual double TTmin() const;
        virtual double TTmax() const;
            
        int getMaxHklSqSum() const;
        double getTextSize() const;
        double getSpotSize() const;
        bool spotsEnabled() const;
        
        int markerNumber() const;
        QPointF getMarkerDetPos(int n) const;
        QList<Vec3D> getMarkerNormals() const;
        
        virtual void projector2xml(QXmlStreamWriter&);
        virtual void loadFromXML(QXmlStreamReader&);
    public slots:
        void connectToCrystal(Crystal *);
        void setWavevectors(double Qmin, double Qmax);
        void reflectionsUpdated();

        void addRotation(const Vec3D &axis, double angle);
        void addRotation(const Mat3D& M);
        void setRotation(const Mat3D& M);

        virtual void decorateScene()=0;
        void setMaxHklSqSum(int m);
        void setTextSize(double d);
        void setSpotSize(double d);
        void enableSpots(bool b=true);
        // For speedup of fitting...
        void enableProjection(bool b=true);
        

        void addMarker(const QPointF& p);
        void delMarkerNear(const QPointF& p);
        
        virtual void doImgRotation(int CWRSteps, bool flip);

        void addInfoItem(const QString& text, const QPointF& p);
        void clearInfoItems();
        
    signals:  
        void projectedPointsUpdated();
        void wavevectorsUpdated();
        void projectionParamsChanged();
        void projectionRectPosChanged();
        void projectionRectSizeChanged();
        void imgTransformUpdated();

    protected:
        virtual bool project(const Reflection &r, QGraphicsItem* item)=0;
        virtual QGraphicsItem* itemFactory()=0;
    
        virtual bool parseXMLElement(QXmlStreamReader&);

        // These are the reflections
        QList<QGraphicsItem*> projectedItems;
        // Stuff like Primary beam marker, Coordinate lines
        QList<QGraphicsItem*> decorationItems;
        // written indexes in the scene
        QList<QGraphicsItem*> textMarkerItems;
        // Markers for indexation and fit
        QList<QGraphicsEllipseItem*> markerItems;
        // Info Items. These will be set on Mousepress from Python and be deleted on orientation change or slot!
        QList<QGraphicsItem*> infoItems;
    
        // Pointer to the connectred crystal
        QPointer<Crystal> crystal;
    
        double QminVal;
        double QmaxVal;
        int maxHklSqSum;
        double textSize;
        double spotSize;
        bool showSpots;
        bool projectionEnabled;
        
        QGraphicsScene scene;
        
        QGraphicsItemGroup imgGroup;
    protected slots:
        virtual void updateImgTransformations();
};

double getDoubleAttrib(QXmlStreamReader &r, QString name, double def);
int getIntAttrib(QXmlStreamReader &r, QString name, double def);

#endif
