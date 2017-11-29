/*
==== Author:

Relja Arandjelovic (relja@robots.ox.ac.uk)
Visual Geometry Group,
Department of Engineering Science
University of Oxford

==== Copyright:

The library belongs to Relja Arandjelovic and the University of Oxford.
No usage or redistribution is allowed without explicit permission.
*/

#include "homography.h"

homography::homography( ellipse const &el1, ellipse const &el2) {
  affine_homography( el1, el2 );
}

void homography::affine_homography( ellipse const &el1, ellipse const &el2) {
    double ma1, mb1, mc1, ma2, mb2, mc2, ma2i, mb2i, mc2i, a, b, c, tx, ty;

    // as in James's RANSAC:
    // A1= C1^T C1
    homography::cholesky( el1.a, el1.b, el1.c, ma1, mb1, mc1 );
    // A2= C2^T C2
    homography::cholesky( el2.a, el2.b, el2.c, ma2, mb2, mc2 );
    
    homography::lowerTriInv( ma2, mb2, mc2, ma2i, mb2i, mc2i );
    
    // H= C2^(-1) C1
    a = ma1*ma2i;
    c = mc1*mc2i;
    b = ma1*mb2i + mb1*mc2i;

    tx = el2.x - a*el1.x;
    ty = el2.y - b*el1.x - c*el1.y;
    
    H[0]= a; H[1]= 0; H[2]= tx;
    H[3]= b; H[4]= c; H[5]= ty;
    H[6]= 0; H[7]= 0; H[8]=  1;
} 

// source: Yujie Zhong (Sep. 2017)
// updated: Abhishek Dutta (Sep. 2017)
void homography::similarity_homography( ellipse const &el1, ellipse const &el2) {
  double area1, area2, s, theta1, theta2, theta, a, b, tx, ty;
  double d1,d2;

  area1 = 1/( el1.a * el1.c - el1.b * el1.b );
  area2 = 1/( el2.a * el2.c - el2.b * el2.b );

  s = sqrt( area2 / area1 );
  d1 = el1.c - el1.a;
  d2 = el2.c - el2.a;

  theta1 = atan( (d1 + sqrt(d1*d1 + 4*el1.b*el1.b)) / (2*el1.b) );
  theta2 = atan( (d2 + sqrt(d2*d2 + 4*el2.b*el2.b)) / (2*el2.b) );

  theta = theta2 - theta1;

  a = s*cos(theta);
  b = s*sin(theta);

  tx = el2.x - a*el1.x + b*el1.y;
  ty = el2.y - b*el1.x - a*el1.y;

  H[0]= a; H[1]=-b; H[2]= tx;
  H[3]= b; H[4]= a; H[5]= ty;
  H[6]= 0; H[7]= 0; H[8]=  1;
}


void
homography::cholesky( double a, double b, double c, double &at, double &bt, double &ct ){
    ct= sqrt(c);
    bt= b/ct;
    at= sqrt(a-bt*bt);
}

void
homography::lowerTriInv( double a, double b, double c, double &at, double &bt, double &ct ){
    double invdet = 1.0/(a*c);
    at=  c*invdet;
    bt= -b*invdet;
    ct=  a*invdet;
}

double
homography::getDetAffine() const {
    if (H[8] < 1e-4) return 0;
    return (H[0]*H[4] - H[1]*H[3])/(H[8]*H[8]);
}



inline double mysqr(double x){ return x*x; }

double homography::getSimEig(){
    return 1 - 4 * mysqr( H[0]*H[4]-H[1]*H[3] ) / mysqr( H[0]*H[0]+H[1]*H[1]+H[3]*H[3]+H[4]*H[4] );
}
