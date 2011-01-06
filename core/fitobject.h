#ifndef __FITOBJECT_H__
#define __FITOBJECT_H__

#include <QObject>
#include <QList>


class FitParameter;

class FitParameterGroup {
public:
  FitParameterGroup() { valuesCached = false; }
  virtual ~FitParameterGroup();
  QList<FitParameter*> parameters() const { return groupParameters; }
  void notifyCached() { valuesCached = true; }
  void setValue();
  virtual double value(int member) const = 0;
  virtual double epsilon(int member) const = 0;
  virtual double lowerBound(int member) const = 0;
  virtual double upperBound(int member) const = 0;
protected:
  void addParameter(QString name);
  virtual void doSetValue(QList<double> values)=0;
  QList<FitParameter*> groupParameters;
  bool valuesCached;
};

class FitParameter {
public:
  FitParameter(QString n, int id, FitParameterGroup& g): _name(n), memberId(id), group(g) { setEnabled(false); setChangeable(true); cachedValue = value(); hasCachedValue=false; }
  QString name() const { return _name; }
  double value() const { return group.value(memberId); }
  void prepareValue(double v) { if (v != value()) { cachedValue=v; group.notifyCached(); hasCachedValue=true; } }
  double getCachedValue() const { if (hasCachedValue) return cachedValue; else return value(); }
  void setValue() { hasCachedValue=false; group.setValue(); }
  double lowerBound() const { return group.lowerBound(memberId); }
  double upperBound() const { return group.upperBound(memberId); }
  double epsilon() const { return group.epsilon(memberId); }
  bool isEnabled() const { return enabled; }
  void setEnabled(bool b) {enabled = b; }
  bool isChangeable() const { return changeable; }
  void setChangeable(bool b) { changeable=b; if (!b) setEnabled(b); }
protected:
  QString _name;
  int memberId;
  bool enabled;
  bool changeable;
  FitParameterGroup& group;
  double cachedValue;
  bool hasCachedValue;
};



class FitObject: public QObject {
  Q_OBJECT
public:
  FitObject(QObject* parent=0): QObject(parent) {}
  virtual ~FitObject() {}
  void addParameterGroup(FitParameterGroup* g);
  QList<FitParameter*> allParameters() const;
  QList<FitParameter*> changeableParameters() const;
  QList<FitParameter*> enabledParameters() const;
protected:
  QList<FitParameterGroup*> groups;

};

#endif
