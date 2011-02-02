#ifndef RESIZEINGTABLEWIDGET_H
#define RESIZEINGTABLEWIDGET_H

#include <QTableWidget>

class ResizingTableWidget : public QTableWidget
{
    Q_OBJECT
public:
    explicit ResizingTableWidget(QWidget *parent = 0);
    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;
signals:

public slots:

};

#endif // RESIZEINGTABLEWIDGET_H
