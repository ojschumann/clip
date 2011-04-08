#ifndef FITPARAMETER_H
#define FITPARAMETER_H

#include <QObject>
#include <QTreeWidgetItem>

#include "refinement/fitparametergroup.h"

class FitParameter: public QObject {
  Q_OBJECT
public:
  FitParameter(QString n, int id, FitParameterGroup& g);
  virtual ~FitParameter() {}
  QString name() const { return _name; }
  double value() const { return group.value(memberId); }
  double error() const { return errorValue; }
  void prepareValue(double);
  double getCachedValue() const;
  FitParameterGroup* getGroup() { return &group; }
  FitObject* getFitObject() { return group.getFitObject(); }

  void setValue(double e=-1) { errorValue=e; group.setValue(); hasCachedValue=false; emitValueChanged(); }
  double lowerBound() const { return group.lowerBound(memberId); }
  double upperBound() const { return group.upperBound(memberId); }
  double epsilon() const { return group.epsilon(memberId); }
  bool isEnabled() const { return enabled && changeable; }
  void setEnabled(bool b) {enabled = b; emit enabledStateChanged(this, b); group.notifySetEnabled(memberId, b);}
  bool isChangeable() const { return changeable; }
  void setChangeable(bool b) { changeable=b; emit changeableStateChanged(this, b); group.notifySetChangeable(memberId, b);}
public slots:
  void emitValueChanged() { emit valueChanged(this, value()); }
signals:
  void valueChanged(FitParameter*, double);
  void enabledStateChanged(FitParameter*, bool);
  void changeableStateChanged(FitParameter*, bool);
protected:
  QString _name;
  int memberId;
  bool enabled;
  bool changeable;
  FitParameterGroup& group;
  double errorValue;
  double cachedValue;
  bool hasCachedValue;
};


#endif // FITPARAMETER_H
