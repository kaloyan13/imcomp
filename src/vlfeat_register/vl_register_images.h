#ifndef _VL_REGISTER_IMAGES_H_
#define _VL_REGISTER_IMAGES_H_

extern "C" {
#include <vl/generic.h>
#include <vl/stringop.h>
#include <vl/pgm.h>
#include <vl/sift.h>
#include <vl/getopt_long.h>
}

#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <limits>
#include <random>
#include <algorithm>

#include <Eigen/Dense>
#include <Eigen/SVD>

#include <boost/filesystem.hpp>

#include <Magick++.h>

using namespace Eigen;

using namespace std;

class vl_register_images {
 public:
  static void register_images(const char image_fn1[], const char image_fn2[],
                              double xl, double xu, double yl, double yu,
                              MatrixXd& H, uint32_t& bestNInliners,
                              const char outFn1[], const char outFn2[], const char outFn2t[],
                              const char diff_image[],
                              const char overlap_image[]);

 private:
  static void compute_sift_features(const string filename, vector<VlSiftKeypoint>& keypoint_list, vector< vector<vl_uint8> >& descriptor_list, bool verbose=false);
  static void get_putative_matches(vector< vector<vl_uint8> >& descriptor_list1, vector< vector<vl_uint8> >& descriptor_list2, std::vector< std::pair<uint32_t, uint32_t> > &putative_matches, float threshold);
  static vector<double> get_pixel_percentile(Magick::Image& img, const vector<unsigned int> percentile);
};


#endif
