#ifndef __FITOBJECT_H__
#define __FITOBJECT_H__

#include <QtCore/QList>
#include <QtCore/QPair>
#include <cmath>


class FitObject {
    public:
        FitObject();
        virtual ~FitObject();
        
        void setFitParameterNames(QList<QString> names);
        
        virtual int fitParameterNumber();
        virtual QString fitParameterName(int n);
        virtual double fitParameterValue(int n);
        virtual void fitParameterSetValue(int n, double val);
        virtual QPair<double, double> fitParameterBounds(int n);
        virtual double fitParameterChangeHint(int n);
        virtual bool fitParameterEnabled(int n);
        virtual void fitParameterSetEnabled(int n, bool enable);
        
    protected:
        struct FitParameter {
            QString name;
            bool enabled;
            QPair<double, double> bounds;
            double changeHint;
        };
        
        QList<FitObject::FitParameter> fitParameter;
};

#endif
