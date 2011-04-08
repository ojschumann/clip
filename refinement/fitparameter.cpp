#include "fitparameter.h"

FitParameter::FitParameter(QString n, int id, FitParameterGroup &g):
    _name(n),
    memberId(id),
    group(g)
{
  setEnabled(false);
  setChangeable(true);
  cachedValue = value();
  hasCachedValue=false;
  errorValue = -1.0;
}

void FitParameter::prepareValue(double v) {
  if (v != value()) {
    cachedValue=v;
    group.notifyCached();
    hasCachedValue=true;
  }
}

double FitParameter::getCachedValue() const {
  if (hasCachedValue) {
    return cachedValue;
  } else {
    return value();
  }
}

