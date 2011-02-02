#include "fitparametergroup.h"

#include "refinement/fitparameter.h"


FitParameterGroup::FitParameterGroup(FitObject *o):
    fitObject(o)
{
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

void FitParameterGroup::groupDataChanged() {
  foreach (FitParameter* p, groupParameters) {
    p->emitValueChanged();
  }
}
