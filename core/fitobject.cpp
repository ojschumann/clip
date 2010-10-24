#include <core/fitobject.h>

FitObject::FitObject():fitParameter() {
}

FitObject::~FitObject() {
}

void FitObject::setFitParameterNames(QList<QString> names) {
    fitParameter.clear();
    for (int i=0; i<names.size(); i++) {
        FitParameter p;
        p.name=names[i];
        p.enabled=false;
        p.bounds=qMakePair((double)-INFINITY, (double)INFINITY);
        p.changeHint=1.0;
        fitParameter << p;
    }
}

int FitObject::fitParameterNumber() {
    return fitParameter.size();
}

QString FitObject::fitParameterName(int n) {
    if (n<fitParameter.size())
        return fitParameter[n].name;
    return "";
}

double FitObject::fitParameterValue(int n) {
    return 0.0;
}

void FitObject::fitParameterSetValue(int n, double val) {
}

QPair<double, double> FitObject::fitParameterBounds(int n) {
    if (n<fitParameter.size())
        return fitParameter[n].bounds;
    return qMakePair((double)-INFINITY, (double)INFINITY);
}

double FitObject::fitParameterChangeHint(int n) {
    if (n<fitParameter.size())
        return fitParameter[n].changeHint;    
    return 1.0;
}

bool FitObject::fitParameterEnabled(int n) {
    if (n<fitParameter.size())
        return fitParameter[n].enabled;
    return false;
}

void FitObject::fitParameterSetEnabled(int n, bool enable) {
    if (n<fitParameter.size())
        fitParameter[n].enabled=enable;
}
