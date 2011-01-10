#include "fittreewidget.h"

#include <QStyledItemDelegate>




FitTreeWidget::FitTreeWidget(QWidget *parent) :
    QTreeWidget(parent)
{
setItemDelegateForColumn(0, new MyDelegate(this));
}

bool FitTreeWidget::edit(const QModelIndex &index, EditTrigger trigger, QEvent *event) {
  //if (index.column()==1)
    return QTreeWidget::edit(index, trigger, event);
  return false;
}


