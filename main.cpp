#include <QtGui/QApplication>
#include <ui/clip.h>

#include <tools/mat3D.h>
#include <tools/vec3D.h>
#include <QTime>
#include <iostream>
#include <cmath>
#include <image/dataproviderfactory.h>

#include "tools/optimalrotation.h"

using namespace std;

int main(int argc, char *argv[]) {

  qreal t = 0.01;
  cout << qRound(t) << " " << static_cast<int>(t) << " " << std::ceil(t) << " " << std::floor(t) << endl;
  t=0.49;
  cout << qRound(t) << " " << static_cast<int>(t) << " " << std::ceil(t) << " " << std::floor(t) << endl;
  t=0.5;
  cout << qRound(t) << " " << static_cast<int>(t) << " " << std::ceil(t) << " " << std::floor(t) << endl;
  t=0.51;
  cout << qRound(t) << " " << static_cast<int>(t) << " " << std::ceil(t) << " " << std::floor(t) << endl;

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


