#ifndef FITPARAMETERTREEITEM_H
#define FITPARAMETERTREEITEM_H

#include <QObject>
#include <QTreeWidgetItem>

#include "refinement/fitparameter.h"

class FitParameterTreeItem : public QObject, public QTreeWidgetItem
{
  Q_OBJECT
public:
  explicit FitParameterTreeItem(FitParameter* p, QTreeWidgetItem* parent);
  virtual QVariant data(int column, int role) const;
  virtual void setData(int column, int role, const QVariant &value);
signals:
protected slots:
  void updateData();
  void changeableStateChanged(FitParameter*, bool);
public slots:
private:
  FitParameter* parameter;
};

#endif // FITPARAMETERTREEITEM_H
