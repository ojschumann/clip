#ifndef __MAT3D_H__
#define __MAT3D_H__


#include <vector>

using namespace std;

class Vec3D;

class Mat3D {
    friend class Vec3D;
 public:
  Mat3D();
  Mat3D(Vec3D v1, Vec3D v2,  Vec3D v3);
  Mat3D(vector<double>M);
  Mat3D(Vec3D axis, double angle);
  Mat3D(const Mat3D* m);
  Mat3D(const Mat3D& m);
  
  Vec3D operator*(const Vec3D& v) const;
  Mat3D operator*(const Mat3D& m) const;
  Mat3D operator*(double a) const;
  Mat3D lmult(const Mat3D& m);
  
  Mat3D operator-(const Mat3D& m) const;
  Mat3D operator+(const Mat3D& m) const;

  Mat3D operator*=(const Mat3D& m);
  Mat3D operator*=(double a);
  Mat3D operator+=(const Mat3D& m);
  Mat3D operator-=(const Mat3D& m);

  
      
  Vec3D operator[](unsigned int i) const;
  double* at(unsigned int i, unsigned int j);
  
  Mat3D orthogonalize() const;
  Mat3D transposed() const;
  void transpose();
  Mat3D inverse() const;
  double sqSum() const;
  double det() const;
  
  // returns Q, Matrix will be overwritten
  Mat3D QR();
  Mat3D QL();
  void upperBidiagonal(Mat3D& L, Mat3D& R);
  void svd(Mat3D& L, Mat3D& R);
 protected:
  double M[3][3];
};


//Mat3D operator*(double a, const Mat3D& m);

#endif
