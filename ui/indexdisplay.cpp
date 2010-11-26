#include "indexdisplay.h"
#include "ui_indexdisplay.h"

#include <iostream>

#include "core/crystal.h"
#include "core/projector.h"

using namespace std;

IndexDisplay::IndexDisplay(Crystal* _c, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Indexing),
    crystal(_c)
{
    ui->setupUi(this);
}

IndexDisplay::~IndexDisplay()
{
    delete ui;
}

void IndexDisplay::on_startButton_clicked()
{
  QList<Vec3D> spotMarkerNormals;
  QList<Vec3D> zoneMarkerNormals;
  foreach (Projector* p, crystal->getConnectedProjectors()) {
    spotMarkerNormals += p->getSpotMarkerNormals();
    zoneMarkerNormals += p->getZoneMarkerNormals();
  }
  cout << "Spot Marker" << endl;
  foreach (Vec3D v, spotMarkerNormals) {
    cout << v.x() << " " << v.y() << " " << v.z() << endl;
  }
  cout << "Zone Marker" << endl;
  foreach (Vec3D v, zoneMarkerNormals) {
    cout << v.x() << " " << v.y() << " " << v.z() << endl;
  }
}
