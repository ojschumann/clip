#include "refinement/fitobject.h"

#include "refinement/fitparameter.h"





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

QList<FitObject*> FitObject::getFitObjects() {
  return QList<FitObject*>() << this;
}
