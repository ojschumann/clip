#ifndef __VEC3D__
#define __VEC3D__


class Mat3D;

class Vec3D {
  public:
    Vec3D();
    Vec3D(double x, double y, double z);
    Vec3D(const double _x[3]);
    Vec3D(const Vec3D* v);

    Vec3D operator+(const Vec3D& v) const;
    Vec3D operator-(const Vec3D& v) const;
    Vec3D operator*(double a) const;
    double operator*(const Vec3D& v) const;
    Vec3D operator%(const Vec3D &v) const;
    Mat3D operator^(const Vec3D &v) const;
    Vec3D operator/(double a) const;

    bool operator==(const Vec3D& v) const;
    bool operator!=(const Vec3D& v) const;
    bool isNull() const;

    Vec3D operator+=(const Vec3D& v);
    Vec3D operator-=(const Vec3D& v);
    Vec3D operator*=(double a);
    Vec3D operator/=(double a);

    double& operator[](unsigned int i);
    double operator[](unsigned int i) const;

    Mat3D outer() const;
    double norm() const;
    double norm_sq() const;
    void normalize();
    Vec3D normalized() const;
    
    double x() const;
    double y() const ;
    double z() const;

  protected:
    double X[3]; 
};

#endif
