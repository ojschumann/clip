#include "resizeingtablewidget.h"

#include <QHeaderView>

ResizingTableWidget::ResizingTableWidget(QWidget *parent) :
    QTableWidget(parent)
{
}

QSize ResizingTableWidget::sizeHint() const {

  int height = horizontalHeader()->sizeHint().height()+rowCount()*verticalHeader()->minimumSectionSize();
  int width = verticalHeader()->sizeHint().width()+columnCount()*horizontalHeader()->minimumSectionSize();
  return QSize(width, height);
}

QSize ResizingTableWidget::minimumSizeHint() const {
  return sizeHint();
}
