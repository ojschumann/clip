#ifndef __LAUEPLANEPROJECTOR_H__
#define __LAUEPLANEPROJECTOR_H__

#include <core/projector.h>

class LauePlaneProjector: public Projector {
    Q_OBJECT
    public:
        LauePlaneProjector(QObject* parent=0);
        LauePlaneProjector(const LauePlaneProjector&);
        virtual QPointF scattered2det(const Vec3D&, bool* b=NULL) const;
        virtual Vec3D det2scattered(const QPointF&, bool* b=NULL) const;
        virtual QPointF normal2det(const Vec3D&, bool* b=NULL) const;
        virtual Vec3D det2normal(const QPointF&, bool* b=NULL) const;
    
        virtual QString configName();
        virtual QString projectorName();
        virtual QString displayName();
        double dist() const;
        double width() const;
        double height() const;
        double omega() const;
        double chi() const;
        double phi() const;
        double xOffset() const;
        double yOffset() const;
        virtual void doImgRotation(unsigned int CWRSteps, bool flip);
        
        virtual double TTmin() const;
        virtual double TTmax() const;
        
        // Functions for fitting parameters
        virtual double fitParameterValue(unsigned int n);
        virtual void fitParameterSetValue(unsigned int n, double val);
            
        virtual void projector2xml(QXmlStreamWriter&);
//        virtual void loadFromXML(QXmlStreamReader&);
    public slots:
        void setDetSize(double dist, double width, double height);
        void setDetOrientation(double omega, double chi, double phi);
        void setDetOffset(double dx, double dy);
        virtual void decorateScene();
        void resizePBMarker();
        void movedPBMarker();
        void updatePBPos();
    protected:
        double maxCos(Vec3D n) const;
    
        virtual bool project(const Reflection &r, QPointF& p);
        virtual bool parseXMLElement(QXmlStreamReader&);
    
        Mat3D localCoordinates;
        double detDist;
        double detWidth;
        double detHeight;
        double detOmega;
        double detChi;
        double detPhi;
        double detDx;
        double detDy;
};

#endif
