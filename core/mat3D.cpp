#include <core/mat3D.h>
#include <core/vec3D.h>

#include <cmath>
#include <stdio.h>
#include <iostream>
#include <string.h>

using namespace std;

Mat3D::Mat3D() {
    zero();
    for (unsigned int i=3; i--; )
        M[i][i]=1.0;
}

Mat3D::Mat3D(const Mat3D* t) {
  memcpy(&M, &(t->M), sizeof(M));
}

Mat3D::Mat3D(const Mat3D& t) {
  memcpy(&M, &(t.M), sizeof(M));
}

Mat3D::Mat3D(Vec3D v1,Vec3D v2,Vec3D v3) {
  for (unsigned int i=3; i--; ) {
    M[i][0]=v1[i];
    M[i][1]=v2[i];
    M[i][2]=v3[i];
  }
}


Mat3D::Mat3D(Vec3D axis, double angle) {
  // from http://www.euclideanspace.com/maths/algebra/matrix/orthogonal/rotation/
  
  double s = sin(angle);
  double c = cos(angle);
  double x = axis[0];
  double y = axis[1];
  double z = axis[2];

  M[0][0]=1.0+(1.0-c)*(x*x-1.0);
  M[1][1]=1.0+(1.0-c)*(y*y-1.0);
  M[2][2]=1.0+(1.0-c)*(z*z-1.0);

  M[0][1]=-z*s+(1.0-c)*x*y;
  M[0][2]= y*s+(1.0-c)*x*z;

  M[1][0]= z*s+(1.0-c)*x*y;
  M[1][2]=-x*s+(1.0-c)*y*z;

  M[2][0]=-y*s+(1.0-c)*x*z;
  M[2][1]= x*s+(1.0-c)*y*z;
}


Mat3D::Mat3D(vector<double> m) {
  for (unsigned int i=3; i--; ) {
    for (unsigned int j=3; j--; ) 
      M[i][j]=m[3*j+i];
  }
}

void Mat3D::zero() {
    size_t i1 = 9*sizeof(double);
    size_t i2 = sizeof(M);
    memset(&M, 0, sizeof(M));
}

Vec3D Mat3D::operator*(const Vec3D& v) const {
  Vec3D r;
  for (unsigned int i=3; i--; ) {
    double tmp=0.0;
    for (unsigned int j=3; j--; ) 
      tmp += M[i][j]*v[j];
    r[i]=tmp;
  }
  return r;
}

Mat3D Mat3D::operator+(const Mat3D& m) const {
  Mat3D r;
  for (unsigned int i=3; i--; ) 
    for (unsigned int j=3; j--; ) 
      r.M[i][j]=M[i][j]+m.M[i][j];
  return r;
}

Mat3D Mat3D::operator-(const Mat3D& m) const {
  Mat3D r;
  for (unsigned int i=3; i--; ) 
    for (unsigned int j=3; j--; ) 
      r.M[i][j]=M[i][j]-m.M[i][j];
  return r;
}

Mat3D Mat3D::operator*(const Mat3D& m) const {
  Mat3D r;
  for (unsigned int i=3; i--; ) 
    for (unsigned int j=3; j--; ) {
      double tmp=0.0;
      for (unsigned int k=3; k--; ) 
	tmp+=M[i][k]*m.M[k][j];
      r.M[i][j]=tmp;
    }
  return r;
}


Mat3D Mat3D::operator*(double a) const {
  Mat3D r(this);
  for (unsigned int i=3; i--; ) 
    for (unsigned int j=3; j--; ) 
      r.M[i][j]*=a;
  return r;
}


Mat3D Mat3D::operator*=(const Mat3D& m) {
  Mat3D r(this);
  for (unsigned int i=3; i--; ) 
    for (unsigned int j=3; j--; ) {
      double tmp=0.0;
      for (unsigned int k=3; k--; ) 
	tmp+=r.M[i][k]*m.M[k][j];
      M[i][j]=tmp;
    }
  return *this;
}

Mat3D Mat3D::lmult(const Mat3D& m) {
  Mat3D r(this);
  for (unsigned int i=3; i--; ) 
    for (unsigned int j=3; j--; ) {
      double tmp=0.0;
      for (unsigned int k=3; k--; ) 
	tmp+=m.M[i][k]*r.M[k][j];
      M[i][j]=tmp;
    }
  return *this;
}

Mat3D Mat3D::operator+=(const Mat3D& m) {
    for (unsigned int i=3; i--; ) {
        for (unsigned int j=3; j--; ) {
            M[i][j]+=m.M[i][j];
        }
    }
    return *this;
}

Mat3D Mat3D::operator-=(const Mat3D& m) {
    for (unsigned int i=3; i--; ) {
        for (unsigned int j=3; j--; ) {
            M[i][j]-=m.M[i][j];
        }
    }
    return *this;
}

Mat3D Mat3D::operator*=(double a) {
  for (unsigned int i=3; i--; ) 
    for (unsigned int j=3; j--; ) 
      M[i][j]*=a;
  return Mat3D(this);
}


Vec3D Mat3D::operator[](unsigned int i) const {
  if (i<3) 
    return Vec3D(M[i]);
  return Vec3D();
}


double& Mat3D::at(unsigned int i, unsigned int j) {
  if ((i<3) && (j<3)) 
    return M[i][j];
  return fault;
}



Mat3D Mat3D::orthogonalize() const {
  Mat3D C=(Mat3D()*3.0-((*this)*transposed()))*0.5;
  return C*(*this);
}


void Mat3D::transpose() {
    for (unsigned int i=3; i--; ) {
        for (unsigned int j=i; j--; ) {
            double t=M[i][j];
            M[i][j]=M[j][i];
            M[j][i]=t;
        }
    }
}
    

Mat3D Mat3D::transposed() const {
  Mat3D r(this);
  r.transpose();
  return r;
}

Mat3D Mat3D::inverse() const {
  Mat3D r;
  double d=this->det();
  if (d!=0.0) {
    d=1.0/d;
    r.M[0][0]=d*(M[1][1]*M[2][2]-M[1][2]*M[2][1]);
    r.M[0][1]=d*(M[2][1]*M[0][2]-M[2][2]*M[0][1]);
    r.M[0][2]=d*(M[0][1]*M[1][2]-M[0][2]*M[1][1]);
    r.M[1][0]=d*(M[1][2]*M[2][0]-M[1][0]*M[2][2]);
    r.M[1][1]=d*(M[2][2]*M[0][0]-M[2][0]*M[0][2]);
    r.M[1][2]=d*(M[0][2]*M[1][0]-M[0][0]*M[1][2]);
    r.M[2][0]=d*(M[1][0]*M[2][1]-M[1][1]*M[2][0]);
    r.M[2][1]=d*(M[2][0]*M[0][1]-M[2][1]*M[0][0]);
    r.M[2][2]=d*(M[0][0]*M[1][1]-M[0][1]*M[1][0]);
  }
  return r;
}


double Mat3D::sqSum() const {
  double s=0.0;
  for (unsigned int i=3; i--; ) 
    for (unsigned int j=3; j--; ) 
      s+=M[i][j]*M[i][j];
  return s;
}

double Mat3D::det() const {
  double d=0.0;
  d+=M[0][0]*M[1][1]*M[2][2];
  d+=M[1][0]*M[2][1]*M[0][2];
  d+=M[2][0]*M[0][1]*M[1][2];
  d-=M[0][0]*M[1][2]*M[2][1];
  d-=M[1][0]*M[2][2]*M[0][1];
  d-=M[2][0]*M[0][2]*M[1][1];
  return d;
}

Mat3D Mat3D::QR() {
    Mat3D Q;
    for (unsigned int n=0; n<2; n++) {
        Vec3D u;
        for (unsigned int i=3-n; i--; ) u[2-i]=M[2-i][n];
        u[n]+=u.norm();
        u.normalize();
        Mat3D Qt(u.outer());
        Qt*=2.0;
        for (unsigned int i=3; i--; ) Qt.M[i][i]-=1.0;
        lmult(Qt);
        Q*=Qt;
    }
    return Q;
}

Mat3D Mat3D::QL() {
    Mat3D Q;
    for (unsigned int n=0; n<2; n++) {
        Vec3D u;
        for (unsigned int i=3-n; i--; ) u[i]=M[i][2-n];
        u[2-n]+=u.norm();
        u.normalize();
        Mat3D Qt(u.outer());
        Qt*=2.0;
        for (unsigned int i=3; i--; ) Qt.M[i][i]-=1.0;
        lmult(Qt);
        Q*=Qt;
    }
    return Q;
}

void Mat3D::upperBidiagonal(Mat3D& L, Mat3D& R) {
    for (unsigned int n=0; n<2; n++) {
        Vec3D u;
        for (unsigned int i=3-n; i--; ) u[2-i]=M[2-i][n];
        u[n]+=u.norm();
        u.normalize();
        Mat3D T(u.outer());
        T*=2.0;
        for (unsigned int i=3; i--; ) T.M[i][i]-=1.0;
        lmult(T);
        L*=T;
        
        if (n<1) {
            for (unsigned int i=3; i--; ) u[i]=(i<=n)?0.0:M[n][i];
            u[n+1]+=u.norm();
            u.normalize();
            T=u.outer();
            T*=2.0;
            for (unsigned int i=3; i--; ) T.M[i][i]-=1.0;
            (*this)*=T;
            R.lmult(T);
        }
    }
}

void givens(double a, double b, double &c, double &s) {
    if (b==0.0) {
        c=1.0;
        s=0.0;
    } else if (fabs(a)>fabs(b)) {
        double t=-b/a;
        c=1.0/sqrt(1.0+t*t);
        s=c*t;
    } else {
        double t=-a/b;
        s=1.0/sqrt(1.0+t*t);
        c=t*s;
    }
}



void Mat3D::svd(Mat3D& L, Mat3D& R) {

    upperBidiagonal(L,R);
    L.transpose();
    
    unsigned int loops=250;
    double sumDiag=0.0;
    double sumOffdiag=0.0;
        
    do {

        // Givens rotation
        // TODO: Could be done without the whole matrix multiplications!!! See GSL
        for (unsigned int n=0; n<2; n++) {
            double c,s;
            givens(M[0][n],M[0][n+1], c, s);
            Mat3D G1;
            G1.M[n][n]=c;
            G1.M[n+1][n+1]=c;
            G1.M[n+1][n]=-s;
            G1.M[n][n+1]=s;
            
            (*this)*=G1;
            R*=G1;
            
            givens(M[n][n], M[n+1][n], c, s);
            Mat3D G2;
            G2.M[n][n]=c;
            G2.M[n+1][n+1]=c;
            G2.M[n][n+1]=-s;
            G2.M[n+1][n]=s;
            
            this->lmult(G2);
            L.lmult(G2);
        }    
        
        // Make singular values positive!
        Mat3D T;
        for (unsigned int n=3; n--; )
            T.M[n][n]=(M[n][n]<0.0)?-1.0:1.0;
        this->lmult(T);
        L.lmult(T);
        
        sumDiag=0.0;
        sumOffdiag=0.0;
        for (unsigned int i=3; i--; ) sumDiag+=fabs(M[i][i]);
        for (unsigned int i=2; i--; ) sumOffdiag+=fabs(M[i][i+1]);
            
    } while (loops-- and sumDiag<1e20*sumOffdiag);
    L.transpose();
    R.transpose();
};    
    
double Mat3D::fault=0.0;

