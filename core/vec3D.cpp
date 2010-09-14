#include <core/vec3D.h>
#include <core/mat3D.h>

#include <cmath>
#include <stdio.h>


using namespace std;

Vec3D::Vec3D(){
  for (unsigned int i=3; i--; ) X[i] = 0.0;
}

Vec3D::Vec3D(double _x, double _y, double _z) {
  X[0]=_x;
  X[1]=_y;
  X[2]=_z;
}

Vec3D::Vec3D(const double _x[3]) {
  for (unsigned int i=3; i--; ) X[i] = _x[i];
}

Vec3D::Vec3D(const Vec3D* v) {
  for (unsigned int i=3; i--; ) X[i] = v->X[i];
}

Vec3D Vec3D::operator+(const Vec3D& v) const{
  Vec3D r(this);
  for (unsigned int i=3; i--; ) r.X[i] += v.X[i];
  return r;
}

Vec3D Vec3D::operator-(const Vec3D& v) const {
  Vec3D r(this);
  for (unsigned int i=3; i--; ) r.X[i] -= v.X[i];
  return r;
}

double Vec3D::operator*(const Vec3D& v) const {
  double p=0.0;
  for (unsigned int i=3; i--; ) p+=X[i]*v.X[i];
  return p;
}

Vec3D Vec3D::operator*(double a) const {
  Vec3D r(this);
  for (unsigned int i=3; i--; ) r.X[i] *= a;
  return r;
}

Vec3D Vec3D::operator%(const Vec3D &v) const {
  Vec3D r;
  r.X[0]=X[1]*v.X[2]-X[2]*v.X[1];
  r.X[1]=X[2]*v.X[0]-X[0]*v.X[2];
  r.X[2]=X[0]*v.X[1]-X[1]*v.X[0];
  return r;
}

Mat3D Vec3D::operator^(const Vec3D &v) const {
    Mat3D M;
    for (int i=3; i--; ) {
        for (int j=3; j--; ) {
            double tmp=X[i]*v.X[j];
            M.M[i][j]=tmp;
        }
    }
    return M;
}

Vec3D Vec3D::operator/(double a)  const {
  if (a==0) return Vec3D(this);
  Vec3D r(this);
  for (unsigned int i=3; i--; ) r.X[i] /= a;
  return r;
}


Vec3D Vec3D::operator+=(const Vec3D& v) {
  for (unsigned int i=3; i--; ) X[i] += v.X[i];
  return Vec3D(this);
}

Vec3D Vec3D::operator-=(const Vec3D& v){
  for (unsigned int i=3; i--; ) X[i] -= v.X[i];
  return Vec3D(this);
}

Vec3D Vec3D::operator*=(double a){
  for (unsigned int i=3; i--; ) X[i] *= a;
  return Vec3D(this);
}

Vec3D Vec3D::operator/=(double a) {
  if (a!=0)
    for (unsigned int i=3; i--; ) X[i] /= a;
  return Vec3D(this);
}


Mat3D Vec3D::outer() const {
    Mat3D M;
    for (unsigned int i=3; i--; ) {
        M.M[i][i]=X[i]*X[i];
        for (unsigned int j=i; j--; ) {
            double tmp=X[i]*X[j];
            M.M[i][j]=tmp;
            M.M[j][i]=tmp;
        }
    }
    return M;
}

double Vec3D::norm()  const {
  return sqrt(X[0]*X[0]+X[1]*X[1]+X[2]*X[2]);
}

double Vec3D::norm_sq()  const {
  return X[0]*X[0]+X[1]*X[1]+X[2]*X[2];
}

void Vec3D::normalize() {
  double n=norm();
  if (n!=0) 
    for (unsigned int i=3; i--; ) X[i] /= n;
}

Vec3D Vec3D::normalized() const {
  Vec3D v=Vec3D(this);
  v.normalize();
  return v;
}

double Vec3D::x()  const {
  return X[0];
}

double Vec3D::y()  const {
  return X[1];
}

double Vec3D::z()  const {
  return X[2];
}

double& Vec3D::operator[](unsigned int i) {
  static double err=0.0;
  if (i<3)
    return X[i];
  return err;
}

double Vec3D::operator[](unsigned int i) const {
  if (i<3)
    return X[i];
  return 0.0;
}


bool Vec3D::operator==(const Vec3D& v)  const {
  return (X[0]==v.X[0]) && (X[1]==v.X[1]) && (X[2]==v.X[2]);
}

bool Vec3D::operator!=(const Vec3D& v)  const {
  return (X[0]!=v.X[0]) || (X[1]!=v.X[1]) || (X[2]!=v.X[2]);
}

bool Vec3D::isNull()  const {
  return (X[0]==0.0) && (X[1]==0.0) && (X[2]==0.0);
}
