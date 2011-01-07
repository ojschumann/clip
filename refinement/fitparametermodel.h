#ifndef FITPARAMETERMODEL_H
#define FITPARAMETERMODEL_H

#include <QAbstractItemModel>
#include <QSignalMapper>

class Crystal;
class FitObject;
class FitParameter;
class Projector;

class FitParameterModel : public QAbstractItemModel
{
  Q_OBJECT
public:
  explicit FitParameterModel(Crystal* c, QObject *parent = 0);
  virtual ~FitParameterModel();
  virtual int columnCount(const QModelIndex &parent) const;
  virtual int rowCount(const QModelIndex &parent) const;
  virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;
  virtual QModelIndex parent(const QModelIndex &child) const;

  virtual Qt::ItemFlags flags(const QModelIndex &index) const;
  virtual QVariant data(const QModelIndex &index, int role) const;
  virtual bool setData(const QModelIndex &index, const QVariant &value, int role);

  FitObject* node(const QModelIndex& index) const;
  FitParameter* parameter(const QModelIndex& index) const;

protected slots:
  void handleProjectorAdd(Projector*);
  void handleProjectorDel(Projector*);

  void handleMarkerAdd();
  void handleMarkerDel();

protected:
  Crystal* crystal;
  QList<FitObject*> nodes;
  QSignalMapper mapper;

};

#endif // FITPARAMETERMODEL_H
