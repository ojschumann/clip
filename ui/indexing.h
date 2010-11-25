#ifndef INDEXING_H
#define INDEXING_H

#include <QWidget>

namespace Ui {
    class Indexing;
}

class Indexing : public QWidget
{
    Q_OBJECT

public:
    explicit Indexing(QWidget *parent = 0);
    ~Indexing();

private:
    Ui::Indexing *ui;
};

#endif // INDEXING_H
