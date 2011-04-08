#ifndef FITPARAMETERGROUP_H
#define FITPARAMETERGROUP_H

#include <QObject>

class FitParameter;
class FitObject;

class FitParameterGroup: public QObject {
  Q_OBJECT
public:
  FitParameterGroup(FitObject* o);
  virtual ~FitParameterGroup();
  QList<FitParameter*> parameters() const { return groupParameters; }
  void notifyCached() { valuesCached = true; }
  virtual void notifySetEnabled(int member, bool b) {}
  virtual void notifySetChangeable(int member, bool b) {}
  void setValue();
  FitObject* getFitObject() { return fitObject; }
  virtual double value(int member) const = 0;
  virtual double epsilon(int member) const = 0;
  virtual double lowerBound(int member) const = 0;
  virtual double upperBound(int member) const = 0;
public slots:
  void groupDataChanged();
protected:
  void addParameter(QString name, bool initiallyEnabled=false);
  virtual void doSetValue(QList<double> values)=0;
  QList<FitParameter*> groupParameters;
  bool valuesCached;
  FitObject* fitObject;
};



#endif // FITPARAMETERGROUP_H
