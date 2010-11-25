#include "indexing.h"
#include "ui_indexing.h"

Indexing::Indexing(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Indexing)
{
    ui->setupUi(this);
}

Indexing::~Indexing()
{
    delete ui;
}
