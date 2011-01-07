#include "refinement/fitobject.h"

FitParameterGroup::FitParameterGroup() {
  valuesCached = false;
}

FitParameterGroup::~FitParameterGroup()  {
  foreach (FitParameter* p, groupParameters)
    delete p;
  groupParameters.clear();
}

void FitParameterGroup::setValue() {
  if (valuesCached) {
    QList<double> values;
    foreach(FitParameter* p, groupParameters)
      values +=p->getCachedValue();
    doSetValue(values);
  }
  valuesCached=false;
}

void FitParameterGroup::addParameter(QString name, bool initiallyEnabled)  {
  groupParameters += new FitParameter(name, groupParameters.size(), *this);
  groupParameters.last()->setEnabled(initiallyEnabled);
}


FitParameter::FitParameter(QString n, int id, FitParameterGroup &g):
    _name(n),
    memberId(id),
    group(g)
{
  setEnabled(false);
  setChangeable(true);
  cachedValue = value();
  hasCachedValue=false;
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




void FitObject::addParameterGroup(FitParameterGroup* g) {
  groups << g;
}

FitObject& FitObject::operator=(const FitObject& o) {
  QList<FitParameter*> tP = allParameters();
  QList<FitParameter*> oP = o.allParameters();
  for (int n=0; n<tP.size(); n++) {
    tP.at(n)->setChangeable(oP.at(n)->isChangeable());
    tP.at(n)->setEnabled(oP.at(n)->isEnabled());
  }
  return *this;
}

QList<FitParameter*> FitObject::allParameters() const {
  QList<FitParameter*>  list;
  foreach (FitParameterGroup* g, groups)
    list += g->parameters();
  return list;
}

QList<FitParameter*> FitObject::changeableParameters() const {
  QList<FitParameter*>  list;
  foreach (FitParameterGroup* g, groups)
    foreach (FitParameter* p, g->parameters())
      if (p->isChangeable())
        list += p;
  return list;
}

QList<FitParameter*> FitObject::enabledParameters() const {
  QList<FitParameter*>  list;
  foreach (FitParameterGroup* g, groups)
    foreach (FitParameter* p, g->parameters())
      if (p->isEnabled())
        list += p;
  return list;
}
