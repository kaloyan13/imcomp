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
#include <fstream>
#include <chrono>  // for high_resolution_clock
#include <sstream>

#include <cmath>

#include <Eigen/Dense>
#include <Eigen/SVD>

#include <boost/filesystem.hpp>

#include <Magick++.h>

#include <imcomp/imcomp_cache.h>

using namespace Eigen;
using namespace std;
using namespace Magick;
using namespace std::chrono;

class imreg_sift {
 public:
  // Applies RANSAC and Direct Linear Transform (DLT)
  // see Chapter 4 of Hartley and Zisserman (2nd Edition)
  static void ransac_dlt(const char im1_fn[], const char im2_fn[],
                         double xl, double xu, double yl, double yu,
                         MatrixXd& Hopt, size_t& fp_match_count,
                         const char im1_crop_fn[], const char im2_crop_fn[], const char im2_tx_fn[],
                         const char diff_image_fn[],
                         const char overlap_image_fn[],
                         bool& success,
                         std::string& message,
                         imcomp_cache* cache,
                         std::string& transform);

  // Applies robust filtering of point correspondences and uses Thin Plate Spline for image registration
  //
  // Robust filtering of point correspondences are based on:
  // Tran, Q.H., Chin, T.J., Carneiro, G., Brown, M.S. and Suter, D., 2012, October. In defence of RANSAC for outlier rejection in deformable registration. ECCV.
  //
  // Thin plate spline registration based on:
  // Bookstein, F.L., 1989. Principal warps: Thin-plate splines and the decomposition of deformations. IEEE TPAMI.
  static void robust_ransac_tps(const char im1_fn[], const char im2_fn[],
                                double xl, double xu, double yl, double yu,
                                MatrixXd& Hopt, size_t& fp_match_count,
                                const char im1_crop_fn[], const char im2_crop_fn[], const char im2_tx_fn[],
                                const char diff_image_fn[],
                                const char overlap_image_fn[],
                                bool& success,
                                std::string& message,
                                imcomp_cache* cache);

 // Computes image features for a give uploaded file with a unique file id
 // and caches the features to imrpove performance.
 static bool cache_img_with_fid(boost::filesystem::path upload_dir, std::string fid, imcomp_cache* cache);

 private:
  // implementation of Direct Linear Transform (DLT)
  // see Chapter 4 of Hartley and Zisserman (2nd Edition)
  static void dlt(const MatrixXd& X, const MatrixXd& Y, Matrix<double,3,3>& H);

  // estimates transformation between two set of M corresponding points X and Y of N dimensions.
  // see the classic Linear Least Square paper: https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=88573
  // The implementation is based on the sklearn implementation of the paper in function "_umeyama" here:
  // https://github.com/scikit-image/scikit-image/blob/master/skimage/transform/_geometric.py
  static void estimate_transform(const MatrixXd& X, const MatrixXd& Y, std::string& transform, Matrix<double,3,3>& T);

  // estimates the photometric transform given two sets of images
  static void estimate_photo_transform(Magick::Image img_one, Magick::Image img_two, Magick::Image& transformed_img);

  // use vlfeat to compute SIFT keypoint and descriptors
  //static void compute_sift_features(const string filename, vector<VlSiftKeypoint>& keypoint_list, vector< vector<vl_uint8> >& descriptor_list, bool verbose=false);
  static void compute_sift_features(const Magick::Image& img, vector<VlSiftKeypoint>& keypoint_list, vector< vector<vl_uint8> >& descriptor_list, bool verbose=false);

  // get putative matches based on Lowe's algorithm
  static void get_putative_matches(vector< vector<vl_uint8> >& descriptor_list1, vector< vector<vl_uint8> >& descriptor_list2, std::vector< std::pair<uint32_t, uint32_t> > &putative_matches, float threshold);

  // utility functions
  static void get_norm_matrix(const MatrixXd& pts, Matrix<double,3,3>& T);
  static vector<double> get_pixel_percentile(Magick::Image& img, const vector<unsigned int> percentile);
  static inline double clamp(double v, double min, double max);
  static void get_diff_image(Magick::Image& im1, Magick::Image& im2, Magick::Image& cdiff);
};


#endif
