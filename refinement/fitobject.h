#ifndef __FITOBJECT_H__
#define __FITOBJECT_H__

#include <QObject>
#include <QList>
#include <QVariant>

class FitParameter;
class FitParameterGroup;


class FitObject: public QObject {
  Q_OBJECT
public:
  FitObject(QObject* parent=0): QObject(parent) {}
  virtual ~FitObject() {}
  virtual void prepareForFit() {}
  virtual FitObject& operator=(const FitObject& o);
  void addParameterGroup(FitParameterGroup* g);
  virtual QString FitObjectName()=0;
  QList<FitParameter*> allParameters() const;
  QList<FitParameter*> changeableParameters() const;
  QList<FitParameter*> enabledParameters() const;
  virtual QList<FitObject*> getFitObjects();
signals:
  void fitObjectAdded(FitObject*);
  void fitObjectRemoved(FitObject*);
protected:
  QList<FitParameterGroup*> groups;

};

Q_DECLARE_METATYPE(FitObject*);

#endif
