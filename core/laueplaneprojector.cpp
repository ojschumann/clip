#include <core/laueplaneprojector.h>
#include <cmath>
#include <QtGui/QGraphicsEllipseItem>
#include <QtGui/QCursor>
#include <tools/signalingellipse.h>
#include <iostream>

using namespace std;

LauePlaneProjector::LauePlaneProjector(QObject* parent): Projector(parent), localCoordinates() {
    setWavevectors(0.0, 4.0*M_1_PI);
    setDetSize(30.0, 110.0, 140.0);
    setDetOrientation(180.0, 0, 0);
    detDx=1.0;
    detDy=1.0;
    setDetOffset(0.0, 0.0);
    
    QList<QString> fitParameterNames;
    fitParameterNames << "Distance" << "X-Offset" << "Y-Offset" << "Omega" << "Chi";
    setFitParameterNames(fitParameterNames);
};

LauePlaneProjector::LauePlaneProjector(const LauePlaneProjector& p): Projector(p) {
    cout << "Laueplaneprojector Copy Constructor" << endl;
    setDetSize(p.dist(), p.width(), p.height());
    setDetOrientation(p.omega(), p.chi(), p.phi());
    detDx=p.detDx;
    detDy=p.detDy;    
};

QPointF LauePlaneProjector::scattered2det(const Vec3D &v, bool* b) const{
    Vec3D w=localCoordinates*v;
    if (w.x()<=0.0) {
        if (b) *b=false;
        return QPointF();
    }
    if (b) *b=true;
    return QPointF(w.y()/w.x()+detDx, w.z()/w.x()+detDy);
}

Vec3D LauePlaneProjector::det2scattered(const QPointF& p, bool* b) const{
    Vec3D v(1.0 , p.x()-detDx, p.y()-detDy);
    v.normalize();
    if (b) *b=true;
    return localCoordinates.transposed()*v;
}

QPointF LauePlaneProjector::normal2det(const Vec3D& n, bool* b) const{
    if (b) {
        Vec3D v(normal2scattered(n, b));
        if (*b) {
            return scattered2det(v,b);
        } else {
            return QPointF();
        }
    }
    return scattered2det(normal2scattered(n));
}


Vec3D LauePlaneProjector::det2normal(const QPointF& p, bool* b)  const {
    if (b) {
        Vec3D v(det2scattered(p, b));
        if (*b) {
            return scattered2normal(v,b);
        } else {
            return Vec3D();
        }
    }
    return scattered2normal(det2scattered(p));
}

void LauePlaneProjector::setDetSize(double dist, double width, double height) {
    if ((detDist!=dist) or (detWidth!=width) or (detHeight!=height)) {
        detDist=dist;
        detWidth=width;
        detHeight=height;
        
        scene.setSceneRect(QRectF(-0.5*detWidth/detDist, -0.5*detHeight/detDist, detWidth/detDist, detHeight/detDist));
        if (projectionEnabled) {
            emit projectionRectSizeChanged();
            emit projectionParamsChanged();
        }
    }
}
    
void LauePlaneProjector::setDetOrientation(double omega, double chi, double phi) {
    if ((detOmega!=omega) or (detChi!=chi) or (detPhi!=phi)) {
        detOmega=omega;
        detChi=chi;
        detPhi=phi;
    
        localCoordinates =Mat3D(Vec3D(0,0,1), M_PI*(omega-180.0)/180.0);
        localCoordinates*=Mat3D(Vec3D(0,1,0), M_PI*chi/180.0);
        localCoordinates*=Mat3D(Vec3D(1,0,0), M_PI*phi/180.0);
        //localCoordinates=Mat3D(Vec3D(0,0,1), M_PI*(omega-180.0)/180.0)*Mat3D(Vec3D(0,1,0), M_PI*chi/180.0)*Mat3D(Vec3D(1,0,0), M_PI*phi/180.0);
        movedPBMarker();
        //emit projectionParamsChanged();
    }
}

void LauePlaneProjector::setDetOffset(double dx, double dy) {
    dx/=dist();
    dy/=dist();
    if ((dx!=detDx) or (dy!=detDy)) {
        detDx=dx;
        detDy=dy;
        updatePBPos();
        if (projectionEnabled)
            emit projectionParamsChanged();
    }
}

bool LauePlaneProjector::project(const Reflection &r, QPointF& p) {
    if (r.lowestDiffOrder==0) 
        return false;

    bool doesReflect=false;
    for (int i=0; i<r.orders.size(); i++) {
        int n=r.orders[i];
        if ((QminVal<=n*r.Qscatter) and (n*r.Qscatter<=QmaxVal)) {
            doesReflect=true;
            break;
        }
    }
    if (not doesReflect)
        return false;
        
    Vec3D v=localCoordinates*r.scatteredRay;
    double s=v.x();
    if (s<1e-10)
        return false;
    
    
    s=1.0/s;
    p.setX(v.y()*s+detDx);
    p.setY(v.z()*s+detDy);
    return true;
}
        
void LauePlaneProjector::decorateScene() {
    while (!decorationItems.empty()) {
        QGraphicsItem* item = decorationItems.takeLast();
        scene.removeItem(item);
        delete item;
    }
    
    QTransform t;
    t.scale(det2img.m11(), det2img.m22());    
    
    SignalingEllipseItem* center=new SignalingEllipseItem(-0.5*spotSize, -0.5*spotSize, spotSize, spotSize, &imgGroup);
    center->setPen(QPen(Qt::red));
    center->setFlag(QGraphicsItem::ItemIsMovable, true);
    center->setCursor(QCursor(Qt::SizeAllCursor));
    center->setTransform(t);
    
    SignalingEllipseItem* handle=new SignalingEllipseItem(-0.5*spotSize, -0.5*spotSize, spotSize, spotSize,center);
    handle->setPen(QPen(Qt::red));
    handle->moveBy(0.1, 0);
    handle->setFlag(QGraphicsItem::ItemIsMovable, true);
    handle->setCursor(QCursor(Qt::SizeAllCursor));
    QGraphicsEllipseItem* marker=new QGraphicsEllipseItem(0.1, 0.1, 0.13, 0.13, center);
    marker->setPen(QPen(Qt::red));
    
    decorationItems.append(center);
    decorationItems.append(handle);
    decorationItems.append(marker);
    
    //center->setPos(0.5, 0.5);
    updatePBPos();
    
    
    connect(center, SIGNAL(positionChanged()), this, SLOT(movedPBMarker()));
    connect(handle, SIGNAL(positionChanged()), this, SLOT(resizePBMarker()));
    resizePBMarker();
}


void LauePlaneProjector::resizePBMarker() {
    if (decorationItems.size()<3)
        return;

    QGraphicsEllipseItem* center=dynamic_cast<QGraphicsEllipseItem*>(decorationItems[0]);
    QGraphicsEllipseItem* handle=dynamic_cast<QGraphicsEllipseItem*>(decorationItems[1]);
    QGraphicsEllipseItem* marker=dynamic_cast<QGraphicsEllipseItem*>(decorationItems[2]);
    
    QPointF p=handle->pos();
    double l=hypot(p.x(), p.y());

    QRectF r(-l, -l, 2*l, 2*l);
    r.moveCenter(center->rect().center());
    marker->setRect(r);
}

void LauePlaneProjector::movedPBMarker() {
    if (decorationItems.size()<3)
        return;
    QGraphicsEllipseItem* center=dynamic_cast<QGraphicsEllipseItem*>(decorationItems[0]);
    QPointF p=center->scenePos();

    bool b;
    QPointF q;
    if (omega()>90.5) {
        detDx=0.0;
        detDy=0.0;
        q=(scattered2det(Vec3D(1,0,0), &b));
    } else if (omega()<89.5) {
        detDx=0.0;
        detDy=0.0;
        q=scattered2det(Vec3D(-1,0,0), &b);
    } else {
        b=false;
    }
    if (b) {
        detDx=p.x()-q.x();
        detDy=p.y()-q.y();
    }
    if (projectionEnabled)
        emit projectionParamsChanged();
}

void LauePlaneProjector::updatePBPos() {
    bool b;
    QPointF q;
    if (omega()>90.5) {
        q=(scattered2det(Vec3D(1,0,0), &b));
    } else if (omega()<89.5) {
        q=scattered2det(Vec3D(-1,0,0), &b);
    } else {
        b=false;
    }
    if (b and decorationItems.size()>2) {
        QGraphicsEllipseItem* center=dynamic_cast<QGraphicsEllipseItem*>(decorationItems[0]);
        center->setPos(det2img.map(q));
    }
}

QString LauePlaneProjector::configName() {
    return QString("LauePlaneCfg");
}

QString LauePlaneProjector::projectorName() {
    return QString("LauePlaneProjector");
}

QString LauePlaneProjector::displayName() {
    return QString("Laue Plane");
}

    
double LauePlaneProjector::dist() const {
    return detDist;
}

double LauePlaneProjector::width() const {
    return detWidth;
}

double LauePlaneProjector::height() const {
    return detHeight;
}
double LauePlaneProjector::omega() const {
    return detOmega;
}

double LauePlaneProjector::chi() const {
    return detChi;
}

double LauePlaneProjector::phi() const {
    return detPhi;
}

double LauePlaneProjector::xOffset() const {
    return detDx*dist();
}

double LauePlaneProjector::yOffset() const {
    return detDy*dist();
}

void LauePlaneProjector::doImgRotation(unsigned int CWRSteps, bool flip) {
    Projector::doImgRotation(CWRSteps, flip);
    if (CWRSteps%2==1) {
        setDetSize(dist(), height(), width());
    }
}





double LauePlaneProjector::fitParameterValue(unsigned int n) {
    switch (n)  {
        case 0:
            return dist();
        case 1:
            return xOffset();
        case 2:
            return yOffset();
        case 3:
            return omega();
        case 4:
            return chi();
    }
    return 0.0;
}

void LauePlaneProjector::fitParameterSetValue(unsigned int n, double val) {
    switch (n)  {
        case 0:
            return setDetSize(val, width(), height());
        case 1:
            return setDetOffset(val, yOffset());
        case 2:
            return setDetOffset(xOffset(), val);;
        case 3:
            return setDetOrientation(val, chi(), phi());
        case 4:
            return setDetOrientation(omega(), val, phi());
    }
}


void LauePlaneProjector::projector2xml(QXmlStreamWriter& w) {
    w.writeStartElement("Projector");
    
    w.writeEmptyElement("DetSize");
    w.writeAttribute("width", QString::number(width()));
    w.writeAttribute("height", QString::number(height()));
    w.writeAttribute("dist", QString::number(dist()));
    
    w.writeEmptyElement("DetOrientation");
    w.writeAttribute("Omega", QString::number(omega()));
    w.writeAttribute("Chi", QString::number(chi()));
    w.writeAttribute("Phi", QString::number(phi()));

    w.writeEmptyElement("DetOffset");
    w.writeAttribute("xOffset", QString::number(xOffset()));
    w.writeAttribute("yOffset", QString::number(yOffset()));
    
    Projector::projector2xml(w);

    w.writeEndElement();
}

bool LauePlaneProjector::parseXMLElement(QXmlStreamReader &r) {
    if (r.name()=="DetSize") {
        setDetSize(getDoubleAttrib(r, "dist", dist()), getDoubleAttrib(r, "width", width()), getDoubleAttrib(r, "height", height()));
        return true;
    } else if (r.name()=="DetOrientation") {
        setDetOrientation(getDoubleAttrib(r, "Omega", omega()), getDoubleAttrib(r, "Chi", chi()), getDoubleAttrib(r, "Phi", phi()));
        return true;
    } else if (r.name()=="DetOffset") {
        setDetOffset(getDoubleAttrib(r, "xOffset", xOffset()), getDoubleAttrib(r, "yOffset", yOffset()));
        return true;
    }
    return Projector::parseXMLElement(r);
}

double LauePlaneProjector::TTmax() const {
    Vec3D n(1.0, 0.0, 0.0);
    double mc=maxCos(n);
    return 180.0-180.0*acos(mc)/M_PI;    
}

double LauePlaneProjector::TTmin() const {
    Vec3D n(-1.0, 0.0, 0.0);
    double mc=maxCos(n);
    return 180.0*acos(mc)/M_PI;    
}


double LauePlaneProjector::maxCos(Vec3D n) const {
    double dx = 0.5*width()/dist();
    double dy = 0.5*height()/dist();

    bool b;
    QPointF p=scattered2det(n, &b);
    if (b and (fabs(p.x())<dx) and (fabs(p.y())<dy))
        return 1.0;
    
    // The four corners of the plane, vectors are normalized!
    QList<Vec3D> corners;
    corners.append(det2scattered(QPointF( dx,  dy)));
    corners.append(det2scattered(QPointF(-dx,  dy)));
    corners.append(det2scattered(QPointF(-dx, -dy)));
    corners.append(det2scattered(QPointF( dx, -dy)));
    
    // small value
    double maxCosTT=-2.0;
    
    for (unsigned int i=0; i<4; i++) {
        // Check one corner
        Vec3D a(corners[i]);
        double cosTT=n*a;
        if (cosTT>maxCosTT) 
            maxCosTT=cosTT;
        
        
        Vec3D b=corners[(i+1)%4]-corners[i];
        double denom = ((n*a)*(b*b)-(n*b)*(a*b));
        if (denom!=0.0) {
            double lmin = ((n*b)*(a*a)-(n*a)*(a*b))/denom;
            if ((lmin>=0.0) and (lmin<=1.0)) {
                Vec3D v=a+b*lmin;
                v.normalize();
                cosTT=n*v;
                if (cosTT>maxCosTT) 
                    maxCosTT=cosTT;
            }
        }
    }
    
    return maxCosTT;
}
