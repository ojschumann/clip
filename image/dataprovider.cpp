#include "dataprovider.h"
#include <iostream>
#include <QTableWidget>
#include <QHeaderView>

#include "ui/resolutioncalculator.h"
#include "ui/contrastcurves.h"

using namespace std;

DataProvider::DataProvider(QObject *parent) :
    QObject(parent)
{
  cout << "init DataProvider" << endl;
}

DataProvider::~DataProvider() {
  cout << "delete DataProvider" << endl;
}

DataProvider* DataProvider::openDevice() {
  return NULL;
}

DataProvider* DataProvider::loadImage(const QString &) {
  return NULL;
}

QString DataProvider::name() {
  if (providerInformation.contains("ImgFilename")) {
    return providerInformation["ImgFilename"].toString();
  }
  return QString();
}

QList<QWidget*> DataProvider::toolboxPages() {
  QList<QWidget*> l;

  // ---------------- Info Table ---------------------------
  QList<QString> keys = providerInformation.keys();
  QTableWidget* table = new QTableWidget(keys.size(),2);
  for (int i=0; i<keys.size(); i++) {
    table->setItem(i, 0, new QTableWidgetItem(keys.at(i)));
    table->setItem(i, 1, new QTableWidgetItem(providerInformation[keys.at(i)].toString()));
    table->item(i,0)->setFlags(Qt::ItemIsEnabled);
    table->item(i,1)->setFlags(Qt::ItemIsEnabled);
  }
  table->setObjectName("Info");
  table->horizontalHeader()->setStretchLastSection(true);
  table->verticalHeader()->setDefaultSectionSize(table->fontMetrics().lineSpacing()+5);
  table->verticalHeader()->hide();
  table->horizontalHeader()->hide();
  l << table;
  // ---------------------------------------------------------
  return l;
}
