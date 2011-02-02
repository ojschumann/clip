#ifndef FITTREEWIDGET_H
#define FITTREEWIDGET_H

#include <QTreeWidget>

class FitTreeWidget : public QTreeWidget
{
  Q_OBJECT
public:
  explicit FitTreeWidget(QWidget *parent = 0);

public slots:
  //virtual void edit(const QModelIndex &index);
protected:
  virtual bool edit(const QModelIndex &index, EditTrigger trigger, QEvent *event);
signals:

public slots:

};

#endif // FITTREEWIDGET_H
