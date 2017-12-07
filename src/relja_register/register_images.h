#ifndef _REGISTER_IMAGES_H_
#define _REGISTER_IMAGES_H_

#include "homography.h"
#include "same_random.h"


class registerImages {
 public:
  static void registerFromGuess(const char image_fn1[], const char image_fn2[],
                                double xl, double xu, double yl, double yu,
                                homography &Hinit, uint32_t& bestNInliners,
                                const char outFn1[], const char outFn2[], const char outFn2t[],
                                const char diff_image[],
                                const char overlap_image[],
                                const char *fullSizeFn1= NULL, const char *fullSizeFn2= NULL );
  static void compute_homography(const char image1_fn[], const char image2_fn[],
                                 double x, double y, double w, double h,
                                 homography &h0);

 private:

  static void
    findBBox2( double xl, double xu, double yl, double yu, homography const &H, double &xl2, double &xu2, double &yl2, double &yu2, uint32_t w2, uint32_t h2 );

};


#endif
