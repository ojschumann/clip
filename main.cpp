#include <QtGui/QApplication>
#include <ui/clip.h>

#include <tools/mat3D.h>
#include <tools/vec3D.h>
#include <QTime>
#include <iostream>
using namespace std;

int main(int argc, char *argv[]) {

  Mat3D M(Vec3D(3,1,8).normalized(), 0.32);
  Vec3D v(8,3,-4);
  int N=1;
  for (int i=0; i<10000000; i++) {
    v = M*v;
  }

  QTime sTime = QTime::currentTime();
  forever {
    for (int i=0; i<1000000; i++) {
      v = M*v;
    }
    if (N%100==0) {
      cout << N << " " << sTime.msecsTo(QTime::currentTime()) << " " << 1.0e9*N/sTime.msecsTo(QTime::currentTime()) << " " << v.norm() << endl;
    }
    N++;
  }


  QApplication a(argc, argv);
  Clip w;
  w.show();
  return a.exec();
}


