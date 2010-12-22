#include <QtGui/QApplication>
#include <ui/clip.h>

#include <tools/mat3D.h>
#include <tools/vec3D.h>
#include <QTime>
#include <iostream>
#include <image/dataproviderfactory.h>

#include "tools/optimalrotation.h"

using namespace std;

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);

  a.setApplicationName("Clip");
  a.setOrganizationDomain("clip.berlios.de");
  a.setOrganizationName("O.J.Schumann");

  Clip* w = Clip::getInstance();
  w->show();
  int r = a.exec();
  Clip::clearInstance();
  return r;
}


