/*

Register an image pair based on SIFT features

Author: Abhishek Dutta <adutta@robots.ox.ac.uk>
Date: 3 Jan. 2018

some code borrowed from: vlfeat-0.9.20/src/sift.c

*/

#include "imreg_sift/imreg_sift.h"

// normalize input points such that their centroid is the coordinate origin (0, 0)
// and their average distance from the origin is sqrt(2).
// pts is 3 x n matrix
void imreg_sift::get_norm_matrix(const MatrixXd& pts, Matrix<double,3,3>& T) {
  Vector3d mu = pts.rowwise().mean();
  MatrixXd norm_pts = (pts).colwise() - mu;

  VectorXd x2 = norm_pts.row(0).array().pow(2);
  VectorXd y2 = norm_pts.row(1).array().pow(2);
  VectorXd dist = (x2 + y2).array().sqrt();
  double scale = sqrt(2) / dist.array().mean();

  T(0,0) = scale; T(0,1) =     0; T(0,2) = -scale * mu(0);
  T(1,0) =     0; T(1,1) = scale; T(1,2) = -scale * mu(1);
  T(2,0) =     0; T(2,1) =     0; T(2,2) = 1;
}

// Implementation of Direct Linear Transform (DLT) algorithm as described in pg. 91
// Multiple View Geometry in Computer Vision, Richard Hartley and Andrew Zisserman, 2nd Edition
// assumption: X and Y are point correspondences. Four 2D points. (4 X 3 in homogenous coordinates)
// X and Y : n x 3 matrix
// H : 3x3 matrix
void imreg_sift::dlt(const MatrixXd& X, const MatrixXd& Y, Matrix<double,3,3>& H) {
  size_t n = X.rows();

  MatrixXd A(2*n, 9);
  A.setZero();
  for(size_t i=0; i<n; ++i) {
    A.row(2*i).block(0,3,1,3) = -Y(i,2) * X.row(i);
    A.row(2*i).block(0,6,1,3) =  Y(i,1) * X.row(i);

    A.row(2*i + 1).block(0,0,1,3) =  Y(i,2) * X.row(i);
    A.row(2*i + 1).block(0,6,1,3) = -Y(i,0) * X.row(i);
  }

  JacobiSVD<MatrixXd> svd(A, ComputeFullV);
  // Caution: the last column of V are reshaped into 3x3 matrix as shown in Pg. 89
  // of Hartley and Zisserman (2nd Edition)
  // svd.matrixV().col(8).array().resize(3,3) is not correct!
  H << svd.matrixV()(0,8), svd.matrixV()(1,8), svd.matrixV()(2,8),
       svd.matrixV()(3,8), svd.matrixV()(4,8), svd.matrixV()(5,8),
                        0,                  0, svd.matrixV()(8,8); // affine transform
//       svd.matrixV()(6,8), svd.matrixV()(7,8), svd.matrixV()(8,8); // projective transform
}


void imreg_sift::estimate_transform(const MatrixXd& X, const MatrixXd& Y, string& transform, Matrix<double,3,3>& T) {

  if (transform == "identity") {
    T << 1.0, 0.0, 0.0,
         0.0, 1.0, 0.0,
         0.0, 0.0, 1.0;
  }

  if (transform == "translation" || transform == "rigid" || transform == "similarity") {
    // ignore homogenous representation
    Matrix<double,4,2> X_, Y_;
    X_ = X.block<4,2>(0,0);
    Y_ = Y.block<4,2>(0,0);

    // example values to test with spot the difference demo image.
    // should give identity rotation
    // X_ <<  -1.1684, -1.34154,
    //         -1.3568, -1.28858,
    //         -1.08083,-1.0768,
    //         -1.47721,-1.02969;
    // Y_  << -1.15706, -1.33154,
    //         -1.35822,-1.28466,
    //         -1.08694, -1.07509,
    //         -1.48525, -1.02964;

    size_t m = X_.cols();
    size_t n = X_.rows();

    Matrix<double,2,4> Xt_, Yt_;
    Xt_ = X_.transpose();
    Yt_ = Y_.transpose();

    // subtract mean from points
    VectorXd X_mu = Xt_.rowwise().mean();
    VectorXd Y_mu = Yt_.rowwise().mean();

    Xt_.colwise() -= X_mu;
    Yt_.colwise() -= Y_mu;
    MatrixXd X_norm = Xt_.transpose();
    MatrixXd Y_norm = Yt_.transpose();

    // compute matrix A first (3 X 3 rotation matrix)
    MatrixXd A = (1.0 / n) * (Y_norm.transpose() * X_norm);
    // rotation matrix to compute
    Matrix<double,2,2> R;
    Matrix<double,2,2> d = Matrix<double,2,2>::Identity();

    // do SVD of A
    JacobiSVD<MatrixXd> svd_u(A, ComputeFullU);
    Matrix<double,2,2> U = svd_u.matrixU();
    U(0,0) = -1.0 * U(0,0);
    U(1,0) = -1.0 * U(1,0);
    JacobiSVD<MatrixXd> svd_v(A, ComputeFullV);
    Matrix<double,2,2> V = svd_v.matrixV();
    V(0,0) = -1.0 * V(0,0);
    V(1,0) = -1.0 * V(1,0);

    if (svd_u.rank() == 0) {
      cout << "SVD leads to 0 rank!" << endl;
      return;
    }

    if (svd_u.rank() == (m - 1)) {
      if ( (U.determinant() * V.determinant() ) > 0) {
        R = U * V;
      } else {
        double s = d(1,1);
        d(1,1) = -1;
        R = U * d * V;
        d(1,1) = s;
      }
    } else {
      R =  U * d * V;
    }

    double scale = 1.0;
    // compute scale if its similarity transform
    if (transform == "similarity") {
      double s_dot_d = svd_u.singularValues().dot(d.diagonal());
      double x_norm_var = X_norm.transpose().cwiseAbs2().rowwise().mean().sum();
      scale = (1.0 / x_norm_var) * (svd_u.singularValues().dot(d.diagonal()));
    }
    // apply scale to rotation and translation
    Vector2d t = Y_mu - (scale * (R * X_mu));
    R = R * scale;

    if (transform == "translation") {
      // apply identity rotation
      T << 1, 0, t(0),
           0, 1, t(1),
           0, 0, 1;
    } else {
      T << R(0,0), R(0,1), t(0),
           R(1,0), R(1,1), t(1),
           0,      0,      1.0;
    }
  }

  if (transform == "affine") {
    // estimate affine transform using DLT algorithm
    dlt(X, Y, T);
  }
  // cout << "Final T value is: " << endl;
  // cout << T << endl;
}

void imreg_sift::compute_photo_transform(Magick::Image img_one, Magick::Image img_two, Magick::Image& transformed_img) {
  int w = img_two.rows();
  int h = img_two.columns();

  int ransac_iters = 10;
  float error_threshold = 0.05;
  int num_pts = min(400, min(w,h));
  int score = 0;
  int max_score = 0;

  std::vector<int> x_pts, y_pts;
  std::vector<int> inlier_x_pts, inlier_y_pts;
  std::vector<int> best_inlier_x_pts, best_inlier_y_pts;

  // initialize random number generator to randomly sample pixels from images
  random_device rand_device;
  mt19937 generator(rand_device());
  uniform_int_distribution<int> x_dist(0, w);
  uniform_int_distribution<int> y_dist(0, h);

  Magick::Color pix = img_one.pixelColor(10,10);

  for (int r = 0; r < ransac_iters; r++) {
    x_pts.clear(); y_pts.clear();
    inlier_x_pts.clear(); inlier_y_pts.clear();
    // pick random x and y points
    for (int i = 0; i < num_pts; i++) {
      x_pts.push_back(x_dist(generator));
      y_pts.push_back(y_dist(generator));
    }

    // select intensities at the random points
    Eigen::VectorXd img_one_red(num_pts), img_two_red(num_pts);
    Eigen::VectorXd img_one_green(num_pts), img_two_green(num_pts);
    Eigen::VectorXd img_one_blue(num_pts), img_two_blue(num_pts);
    for (int i = 0; i < num_pts; i++) {
      Magick::ColorRGB img_one_px(img_one.pixelColor(x_pts[i], y_pts[i]));
      img_one_blue[i] = img_one_px.blue();
      img_one_red[i] = img_one_px.red();
      img_one_green[i] = img_one_px.green();

      // cout << img_one_red[i] << " " << img_one_green[i] << " " << img_one_blue[i] << endl;

      Magick::ColorRGB img_two_px(img_two.pixelColor(x_pts[i], y_pts[i]));
      img_two_blue[i] = img_two_px.blue();
      img_two_red[i] = img_two_px.red();
      img_two_green[i] = img_two_px.green();
    }

    // estimate photometric transform
    Matrix3d A = Matrix3d::Identity();
    Vector3d b;
    VectorXd a_b;

    solve_lse(img_one_red, img_two_red, a_b);
    A(0,0) = a_b(0);
    b(0) = a_b(1);

    solve_lse(img_one_green, img_two_green, a_b);
    A(1,1) = a_b(0);
    b(1) = a_b(1);

    solve_lse(img_one_blue, img_two_blue, a_b);
    A(2,2) = a_b(0);
    b(2) = a_b(1);

    MatrixXd B = b.transpose().replicate(num_pts, 1);
    // cout << "Computed A amd B are: " << endl;
    // cout << A << endl;
    // cout << B << endl;

    MatrixXd iter_img_one = MatrixXd::Ones(num_pts, 3);
    MatrixXd iter_img_two = MatrixXd::Ones(num_pts, 3);
    iter_img_one.col(0) << img_one_red;
    iter_img_one.col(1) << img_one_green;
    iter_img_one.col(2) << img_one_blue;
    iter_img_two.col(0) << img_two_red;
    iter_img_two.col(1) << img_two_green;
    iter_img_two.col(2) << img_two_blue;

    // compute image one from estimate
    MatrixXd computed_img_one = (iter_img_two * A) + B;
    VectorXd errors = (iter_img_one - computed_img_one).cwiseAbs2().rowwise().sum().cwiseSqrt();

    for(int k = 0; k < errors.size(); k++) {
      // cout << errors(k) << endl;
      if (errors(k) < error_threshold) {
        inlier_x_pts.push_back(x_pts[k]);
        inlier_y_pts.push_back(y_pts[k]);
      }
    }
    score = inlier_x_pts.size();

    if (score > max_score) {
      best_inlier_x_pts.clear();
      best_inlier_y_pts.clear();
      for (int j=0; j < inlier_x_pts.size(); j++) {
        best_inlier_x_pts.push_back(inlier_x_pts[j]);
        best_inlier_y_pts.push_back(inlier_y_pts[j]);
      }
    }
    max_score = best_inlier_x_pts.size();
  } // end of ransac loop

  // cout << "best inliers are: " << endl;
  // cout << best_inlier_x_pts.size() << endl;

  if (max_score < 2) {
    cout << "Less than 2 points in best inliers. Returning!" << endl;
    return;
  }

  // select intensities at the best inlier points
  int best_num_pts = best_inlier_x_pts.size();
  Eigen::VectorXd img_one_red(best_num_pts), img_two_red(best_num_pts);
  Eigen::VectorXd img_one_green(best_num_pts), img_two_green(best_num_pts);
  Eigen::VectorXd img_one_blue(best_num_pts), img_two_blue(best_num_pts);
  img_one.modifyImage(); img_two.modifyImage(); transformed_img.modifyImage();

  for (int i = 0; i < best_num_pts; i++) {
    Magick::ColorRGB img_one_px(img_one.pixelColor(best_inlier_x_pts[i], best_inlier_y_pts[i]));
    img_one_blue[i] = img_one_px.blue();
    img_one_green[i] = img_one_px.green();
    img_one_red[i] = img_one_px.red();

    Magick::ColorRGB img_two_px(img_two.pixelColor(best_inlier_x_pts[i], best_inlier_y_pts[i]));
    img_two_blue[i] = img_two_px.blue();
    img_two_green[i] = img_two_px.green();
    img_two_red[i] = img_two_px.red();
  }

  // estimate photometric transform
  Matrix3d A = Matrix3d::Identity();
  Vector3d b;
  VectorXd a_b;

  solve_lse(img_one_red, img_two_red, a_b);
  A(0,0) = a_b(0);
  b(0) = a_b(1);

  solve_lse(img_one_green, img_two_green, a_b);
  A(1,1) = a_b(0);
  b(1) = a_b(1);

  solve_lse(img_one_blue, img_two_blue, a_b);
  A(2,2) = a_b(0);
  b(2) = a_b(1);

  MatrixXd B = b.transpose().replicate(num_pts, 1);

  // cout << "Final A and B values are: " << endl;
  // cout << A << endl;
  // cout << B << endl;

  for(unsigned int ii = 0; ii < transformed_img.rows(); ii++) {
    for(unsigned int jj = 0; jj < transformed_img.columns(); jj++) {
      Magick::ColorRGB img_two_pixel(img_two.pixelColor(jj, ii));
      // apply the computed transform
      double r = (img_two_pixel.red() * A(0,0)) + B(0,0);
      double g = (img_two_pixel.green() * A(1,1)) +  B(0,1);
      double b = (img_two_pixel.blue() * A(2,2)) + B(0,2);
      img_two_pixel.red(r);
      img_two_pixel.green(g);
      img_two_pixel.blue(b);
      transformed_img.pixelColor(jj,ii,img_two_pixel);
    }
  }

  // cout << "done with photometric transform!" << endl;
  // transformed_img.write("/home/shrinivasan/Desktop/res_test.jpg");
  return;
}


void imreg_sift::solve_lse(Eigen::VectorXd x, Eigen::VectorXd y, VectorXd& a_and_b) {
  MatrixXd S = MatrixXd::Ones(y.size(), 2);
  double tolerance = 1e-4;
  S.col(0) << y;
  JacobiSVD<MatrixXd> svd(S, ComputeThinU | ComputeThinV);
  MatrixXd singular_values = svd.singularValues();
  MatrixXd singular_values_inv(S.rows(), S.cols());
  singular_values_inv.setZero();

  for (unsigned int i = 0; i < singular_values.rows(); ++i) {
    if (singular_values(i) > tolerance) {
        singular_values_inv(i, i) = 1. / singular_values(i);
    } else {
        singular_values_inv(i, i) = 0.;
    }
  }
  a_and_b = (svd.matrixU() * singular_values_inv * svd.matrixV().transpose()).transpose() * x;
}

double imreg_sift::clamp(double v, double min, double max) {
  if( v > min ) {
    if( v < max ) {
      return v;
    } else {
      return max;
    }
  } else {
    return min;
  }
}

void imreg_sift::compute_sift_features(const Magick::Image& img,
                                       vector<VlSiftKeypoint>& keypoint_list,
                                       vector< vector<vl_uint8> >& descriptor_list,
                                       bool verbose) {
  vl_bool  err    = VL_ERR_OK ;

  // algorithm parameters based on vlfeat-0.9.20/toolbox/sift/vl_sift.c
  int                O     = - 1 ;
  int                S     =   3 ;
  int                o_min =   0 ;

  double             edge_thresh = -1 ;
  double             peak_thresh = -1 ;
  double             norm_thresh = -1 ;
  double             magnif      = -1 ;
  double             window_size = -1 ;
  int                ndescriptors = 0;

  vl_sift_pix     *fdata = 0 ;
  vl_size          q ;
  int              i ;
  vl_bool          first ;

  double           *ikeys = 0 ;
  int              nikeys = 0, ikeys_size = 0 ;

  // move image data to fdata for processing by vl_sift
  // @todo: optimize and avoid this overhead

  fdata = (vl_sift_pix *) malloc( img.rows() * img.columns() * sizeof(vl_sift_pix) ) ;
  if( fdata == NULL ) {
    cout << "\nfailed to allocated memory for vl_sift_pix array" << flush;
    return;
  }

  size_t flat_index = 0;
  for( unsigned int i=0; i<img.rows(); ++i ) {
    for( unsigned int j=0; j<img.columns(); ++j ) {
      Magick::ColorGray c = img.pixelColor( j, i );
      fdata[flat_index] = c.shade();
      ++flat_index;
    }
  }

  // create filter
  VlSiftFilt *filt = 0 ;

  filt = vl_sift_new (img.columns(), img.rows(), O, S, o_min) ;

  if (peak_thresh >= 0) vl_sift_set_peak_thresh (filt, peak_thresh) ;
  if (edge_thresh >= 0) vl_sift_set_edge_thresh (filt, edge_thresh) ;
  if (norm_thresh >= 0) vl_sift_set_norm_thresh (filt, norm_thresh) ;
  if (magnif      >= 0) vl_sift_set_magnif      (filt, magnif) ;
  if (window_size >= 0) vl_sift_set_window_size (filt, window_size) ;

  if (!filt) {
    cout << "\nCould not create SIFT filter." << flush;
    goto done ;
  }

  /* ...............................................................
   * Process each octave
   * ............................................................ */
  i     = 0 ;
  first = 1 ;
  descriptor_list.clear();
  keypoint_list.clear();
  while (1) {
    VlSiftKeypoint const *keys = 0 ;
    int                   nkeys ;

    /* calculate the GSS for the next octave .................... */
    if (first) {
      first = 0 ;
      err = vl_sift_process_first_octave (filt, fdata) ;
    } else {
      err = vl_sift_process_next_octave  (filt) ;
    }

    if (err) {
      err = VL_ERR_OK ;
      break ;
    }

    /* run detector ............................................. */
    vl_sift_detect(filt) ;
    keys  = vl_sift_get_keypoints(filt) ;
    nkeys = vl_sift_get_nkeypoints(filt) ;
    i     = 0 ;

    /* for each keypoint ........................................ */
    for (; i < nkeys ; ++i) {
      double                angles [4] ;
      int                   nangles ;
      VlSiftKeypoint const *k ;

      /* obtain keypoint orientations ........................... */
      k = keys + i ;

      VlSiftKeypoint key_data = *k;

      nangles = vl_sift_calc_keypoint_orientations(filt, angles, k) ;

      vl_uint8 d[128]; // added by @adutta
      /* for each orientation ................................... */
      for (q = 0 ; q < (unsigned) nangles ; ++q) {
        vl_sift_pix descr[128];

        /* compute descriptor (if necessary) */
        vl_sift_calc_keypoint_descriptor(filt, descr, k, angles[q]) ;

        vector<vl_uint8> descriptor(128);
        int j;
        for( j=0; j<128; ++j ) {
          float value = 512.0 * descr[j];
          value = ( value < 255.0F ) ? value : 255.0F;
          descriptor[j] = (vl_uint8) value;
          d[j] = (vl_uint8) value;
        }
        descriptor_list.push_back(descriptor);
        ++ndescriptors;

        keypoint_list.push_back(key_data); // add corresponding keypoint
      }
    }
  }

  done :
    /* release filter */
    if (filt) {
      vl_sift_delete (filt) ;
      filt = 0 ;
    }

    /* release image data */
    if (fdata) {
      free (fdata) ;
      fdata = 0 ;
    }
}

void imreg_sift::get_putative_matches(vector< vector<vl_uint8> >& descriptor_list1,
                                      vector< vector<vl_uint8> >& descriptor_list2,
                                      std::vector< std::pair<uint32_t,uint32_t> >& putative_matches,
                                      float threshold) {
  size_t n1 = descriptor_list1.size();
  size_t n2 = descriptor_list2.size();

  putative_matches.clear();
  for( uint32_t i=0; i<n1; i++ ) {
    unsigned int dist_best1 = numeric_limits<unsigned int>::max();
    unsigned int dist_best2 = numeric_limits<unsigned int>::max();
    uint32_t dist_best1_index = -1;

    for( uint32_t j=0; j<n2; j++ ) {
      unsigned int dist = 0;
      for( int d=0; d<128; d++ ) {
        int del = descriptor_list1[i][d] - descriptor_list2[j][d];
        dist += del*del;
        if (dist >= dist_best2) {
          break;
        }
      }

      // find the nearest and second nearest point in descriptor_list2
      if( dist < dist_best1 ) {
        dist_best2 = dist_best1;
        dist_best1 = dist;
        dist_best1_index = j;
      } else {
        if( dist < dist_best2 ) {
          dist_best2 = dist;
        }
      }
    }

    // use Lowe's 2nd nearest neighbour test
    float d1 = threshold * (float) dist_best1;

    if( (d1 < (float) dist_best2) && dist_best1_index != -1 ) {
      putative_matches.push_back( std::make_pair(i, dist_best1_index) );
    }
  }
}

void imreg_sift::get_diff_image(Magick::Image& im1, Magick::Image& im2, Magick::Image& cdiff) {
  Magick::Image im1_gray = im1;
  Magick::Image im2_gray = im2;
  im1_gray.type(Magick::GrayscaleType);
  im2_gray.type(Magick::GrayscaleType);

  // difference between im1 and im2 is red Channel
  // difference between im2 and im1 is blue channel
  // green channel is then just the gray scale image
  Magick::Image diff_gray(im1_gray);
  diff_gray.composite(im1_gray, 0, 0, Magick::AtopCompositeOp);
  diff_gray.composite(im2_gray, 0, 0, Magick::AtopCompositeOp);

  cdiff.composite(diff_gray, 0, 0, Magick::CopyGreenCompositeOp);
  cdiff.composite(im1_gray, 0, 0, Magick::CopyRedCompositeOp);
  cdiff.composite(im2_gray, 0, 0, Magick::CopyBlueCompositeOp);
}

bool imreg_sift::cache_img_with_fid(boost::filesystem::path upload_dir, string fid, imcomp_cache* cache) {
  // compute sift features for the selected file
  boost::filesystem::path fn = upload_dir / ( fid + ".jpg");
  Magick::Image im; im.read( fn.string() );
  im.type(Magick::TrueColorType);

  // cache the image features
  vector<VlSiftKeypoint> keypoints;
  vector< vector<vl_uint8> > descriptors;
  bool is_cached = cache->get(fid, keypoints, descriptors);
  if ( !is_cached ) {
    compute_sift_features(im, keypoints, descriptors, false);
    cache->put(fid, keypoints, descriptors);
    cout << "\n Successfully cached fid: " << fid << "features! " << flush;
    cout << "\n first keypoint is: " << keypoints.at(1).x << " " << keypoints.at(1).y << flush;
  }

  return true;
}

void imreg_sift::ransac_dlt(const char im1_fn[], const char im2_fn[],
                            double xl, double xu, double yl, double yu,
                            MatrixXd& Hopt, size_t& fp_match_count,
                            const char im1_crop_fn[], const char im2_crop_fn[], const char im2_tx_fn[],
                            const char diff_image_fn[],
                            const char overlap_image_fn[],
                            bool& success,
                            std::string& message,
                            imcomp_cache * cache,
                            std::string& transform,
                            bool is_photometric) {
  try {
    high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

    success = false;
    message = "";

    string im1_file_name(im1_fn);
    string im2_file_name(im2_fn);

    string base_filename1 = im1_file_name.substr(im1_file_name.find_last_of("/\\") + 1);
    std::string::size_type const p1(base_filename1.find_last_of('.'));
    base_filename1 = base_filename1.substr(0, p1);

    string base_filename2 = im2_file_name.substr(im2_file_name.find_last_of("/\\") + 1);
    std::string::size_type const p2(base_filename2.find_last_of('.'));
    base_filename2 = base_filename2.substr(0, p2);

    Magick::Image im1; im1.read( im1_fn );
    Magick::Image im2; im2.read( im2_fn );

    // to ensure that image pixel values are 8bit RGB
    im1.type(Magick::TrueColorType);
    im2.type(Magick::TrueColorType);

    // the photometric transformed image
    Magick::Image im2_pt(im2.size(), "black");
    im2_pt.type(Magick::TrueColorType);

    if (is_photometric) {
      compute_photo_transform(im1, im2, im2_pt);
    }

    Magick::Image im1_g = im1;
    im1_g.crop( Magick::Geometry(xu-xl, yu-yl, xl, yl) );

    Magick::Image im2_g = im2;
    // use the photometric transformed image for geometric transform estimation
    // Magick::Image im2_g = im2_pt;

    vector<VlSiftKeypoint> keypoint_list1, keypoint_list2;
    vector< vector<vl_uint8> > descriptor_list1, descriptor_list2;

    high_resolution_clock::time_point before_sift = std::chrono::high_resolution_clock::now();
    //cout << "time before sift computation: " << (duration_cast<duration<double>>(before_sift - start)).count() << endl;

    // check cache before computing features
    // TODO: Enable when we have a proper implementation of caching mechanism.
    // Disabled for now.
    // bool is_im1_cached = cache->get(base_filename1, keypoint_list1, descriptor_list1);
    // if (is_im1_cached) {
    //   cout << "using cached data for fid: " << base_filename1 << endl;
    // }
    // if (!is_im1_cached) {
    //   // images are converted to gray scale before processing
    //   compute_sift_features(im1_g, keypoint_list1, descriptor_list1, false);
    //   // cache for future use
    //   cache->put(base_filename1, keypoint_list1, descriptor_list1);
    //   cout << "successfully cached fid: " << base_filename1 << endl;
    // }
    // cout << "cached first keypoint is: " << keypoint_list1.at(1).x << " " << keypoint_list1.at(1).y << endl;
    //
    // bool is_im2_cached = cache->get(base_filename2, keypoint_list2, descriptor_list2);
    // if (is_im2_cached) {
    //   cout << "using cached data for fid: " << base_filename2 << endl;
    // }
    // if (!is_im2_cached) {
    //   // images are converted to gray scale before processing
    //   compute_sift_features(im2_g, keypoint_list2, descriptor_list2, false);
    //   // cache for future use
    //   cache->put(base_filename2, keypoint_list2, descriptor_list2);
    //   cout << "successfully cached fid: " << base_filename2 << endl;
    // }
    // cout << "second keypoint is: " << keypoint_list2.at(1).x << " " << keypoint_list2.at(1).y << endl;

    // compute SIFT features without caching
    compute_sift_features(im1_g, keypoint_list1, descriptor_list1, false);
    compute_sift_features(im2_g, keypoint_list2, descriptor_list2, false);

    // use Lowe's 2nd nn test to find putative matches
    float threshold = 1.5f;
    std::vector< std::pair<uint32_t, uint32_t> > putative_matches;
    get_putative_matches(descriptor_list1, descriptor_list2, putative_matches, threshold);
    size_t n_match = putative_matches.size();
    fp_match_count = n_match;
    // cout << "Putative matches (using Lowe's 2nd NN test) = " << n_match << endl;
    // high_resolution_clock::time_point after_putative_match = std::chrono::high_resolution_clock::now();
    // cout << "time after putative match computation: " << (duration_cast<duration<double>>(after_putative_match - after_second_sift)).count() << endl;

    if( n_match < 9 ) {
      message = "Number of feature points that match are very low";
      return;
    }

    // Normalize points so that centroid lies at origin and mean distance to
    // original points in sqrt(2)
    MatrixXd im1_match_kp(3, n_match);
    MatrixXd im2_match_kp(3, n_match);

    for( size_t i=0; i<n_match; ++i) {
      VlSiftKeypoint kp1 = keypoint_list1.at( putative_matches[i].first );
      VlSiftKeypoint kp2 = keypoint_list2.at( putative_matches[i].second );
      im1_match_kp(0, i) = kp1.x;
      im1_match_kp(1, i) = kp1.y;
      im1_match_kp(2, i) = 1.0;
      im2_match_kp(0, i) = kp2.x;
      im2_match_kp(1, i) = kp2.y;
      im2_match_kp(2, i) = 1.0;
    }

    Matrix<double,3,3> im1_match_kp_tform, im2_match_kp_tform;
    get_norm_matrix(im1_match_kp, im1_match_kp_tform);
    get_norm_matrix(im2_match_kp, im2_match_kp_tform);
    MatrixXd im2_match_kp_tform_inv = im2_match_kp_tform.inverse();

    MatrixXd im1_match_norm = im1_match_kp_tform * im1_match_kp;
    MatrixXd im2_match_norm = im2_match_kp_tform * im2_match_kp;
    //cout << "im1_match_kp_tform=" << im1_match_kp_tform << endl;
    //cout << "im2_match_kp_tform=" << im2_match_kp_tform << endl;

    // memory cleanup as these are not required anymore
    im1_match_kp.resize(0,0);
    im2_match_kp.resize(0,0);

    // initialize random number generator to randomly sample putative_matches
    random_device rand_device;
    mt19937 generator(rand_device());
    uniform_int_distribution<> dist(0, n_match-1);

    // estimate homography using RANSAC
    size_t max_score = 0;
    Matrix<double,3,3> Hi;
    vector<unsigned int> best_inliers_index;

    // see Hartley and Zisserman p.119
    // in the original image 2 domain, error = sqrt(5.99) * 1 ~ 3 (for SD of 1 pixel error)
    // in the normalized image 2 domain, we have to transform this 3 pixel to normalized coordinates
    double geom_err_threshold_norm = im2_match_kp_tform(0,0) * 3;
    size_t RANSAC_ITER_COUNT = (size_t) ((double) n_match * 0.6);
    for( unsigned int iter=0; iter<RANSAC_ITER_COUNT; iter++ ) {
      // cout << "==========================[ iter=" << iter << " ]==============================" << endl;

      // randomly select 4 matches from putative_matches
      int kp_id1 = dist(generator);
      int kp_id2 = dist(generator);
      int kp_id3 = dist(generator);
      int kp_id4 = dist(generator);
      //cout << "Random entries from putative_matches: " << kp_id1 << "," << kp_id2 << "," << kp_id3 << "," << kp_id4 << endl;

      MatrixXd X(4,3);
      X.row(0) = im1_match_norm.col(kp_id1).transpose();
      X.row(1) = im1_match_norm.col(kp_id2).transpose();
      X.row(2) = im1_match_norm.col(kp_id3).transpose();
      X.row(3) = im1_match_norm.col(kp_id4).transpose();

      MatrixXd Y(4,3);
      Y.row(0) = im2_match_norm.col(kp_id1).transpose();
      Y.row(1) = im2_match_norm.col(kp_id2).transpose();
      Y.row(2) = im2_match_norm.col(kp_id3).transpose();
      Y.row(3) = im2_match_norm.col(kp_id4).transpose();

      // dlt(X, Y, Hi);
      estimate_transform(X, Y, transform, Hi);

      size_t score = 0;
      vector<unsigned int> inliers_index;
      inliers_index.reserve(n_match);
      MatrixXd im1tx_norm = Hi * im1_match_norm; // 3 x n

      im1tx_norm.row(0) = im1tx_norm.row(0).array() / im1tx_norm.row(2).array();
      im1tx_norm.row(1) = im1tx_norm.row(1).array() / im1tx_norm.row(2).array();

      MatrixXd del(2,n_match);
      del.row(0) = im1tx_norm.row(0) - im2_match_norm.row(0);
      del.row(1) = im1tx_norm.row(1) - im2_match_norm.row(1);
      del = del.array().pow(2);
      VectorXd error = del.row(0) + del.row(1);
      error = error.array().sqrt();

      for( size_t k = 0; k < n_match; k++ ) {
        if(error(k) < geom_err_threshold_norm) {
          score++;
          inliers_index.push_back(k);
        }
      }
      // cout << "iter " << iter << " of " << RANSAC_ITER_COUNT << " : score=" << score << ", max_score=" << max_score << ", total_matches=" << putative_matches.size() << endl;
      if( score > max_score ) {
        max_score = score;
        best_inliers_index.swap(inliers_index);
      }
    } // end RANSAC_ITER_COUNT loop

    // high_resolution_clock::time_point after_ransac = std::chrono::high_resolution_clock::now();
    // cout << "after ransac is: " << (duration_cast<duration<double>>(after_ransac - after_putative_match)).count() << endl;

    if(transform != "identity" && max_score < 3 ) {
      message = "Failed to find a suitable transformation";
      return;
    }

    // Recompute homography using all the inliers
    // This does not improve the registration
    // cout << "re-computing homography using inliers" << endl;
    size_t n_inliers = best_inliers_index.size();
    MatrixXd X(n_inliers,3);
    MatrixXd Y(n_inliers,3);
    for( size_t i=0; i<n_inliers; ++i ) {
      X.row(i) = im1_match_norm.col( best_inliers_index.at(i) ).transpose();
      Y.row(i) = im2_match_norm.col( best_inliers_index.at(i) ).transpose();
    }

    Matrix<double, 3, 3> Hopt_norm, H;

    // dlt(X, Y, Hopt_norm);
    //cout << "estimateing transform: " << transform << endl;
    estimate_transform(X, Y, transform, Hopt_norm);

    // see Hartley and Zisserman p.109
    H = im2_match_kp_tform_inv * Hopt_norm * im1_match_kp_tform;
    H = H / H(2,2);
    // Hopt is the reference variable passed to this method
    Hopt = H;

    // im1 crop
    Magick::Image im1_crop(im1);
    Magick::Geometry cropRect1(xu-xl, yu-yl, xl, yl);
    im1_crop.crop( cropRect1 );
    im1_crop.write( im1_crop_fn );

    Magick::Image im2_crop(im2);
    im2_crop.crop( cropRect1 );
    im2_crop.write( im2_crop_fn );

    // ----------------------------------------------------------------
    // apply Homography to im2 in order to visualize it along with im1.
    // ----------------------------------------------------------------
    Matrix<double, 3, 3> Hinv = H.inverse();
    // if we are doing photometric, use photometric transformed image
    // else just the image 2.
    Magick::Image im2t_crop;
    if (is_photometric) {
      im2t_crop = im2_pt;
    } else {
      im2t_crop = im2;
    }

    // some simple affine tralsformations for testing:
    // Magick::DrawableAffine affine(1.0,  1.0, 0, 0, 0.0, 0.0);
    // Magick::DrawableAffine hinv_affine(1,1,.3,0,0,0); // simple shearing
    // Magick::DrawableAffine hinv_affine(1,-1,0,0,0,0); // flip
    // Magick::DrawableAffine hinv_affine(1,0.5,0,0,0,0); // scale
    // set rotation as per Hinv
    Magick::DrawableAffine hinv_affine(Hinv(0,0), Hinv(1,1), Hinv(1,0), Hinv(0,1), 0, 0);

    // set translation and resizing using viewport function.
    // See: http://www.imagemagick.org/discourse-server/viewtopic.php?f=27&t=33029#p157520.
    // As we need to render the image on a webpage canvas, we need to resize the
    // width and height of im2. Then offset in x and y direction based on the translation
    // computed by Homography (H and Hinv matrices).
    string op_w_h_and_offsets = std::to_string(xu-xl) +
                                "x" +
                                std::to_string(yu-yl) +
                                "-" + to_string((int) Hinv(0,2)) +
                                "-" + to_string((int) Hinv(1,2));
    im2t_crop.artifact("distort:viewport", op_w_h_and_offsets);
    im2t_crop.affineTransform(hinv_affine);
    //cout << "done applying Affine Transform" << endl;

    // save result
    im2t_crop.write(im2_tx_fn);
    // TODO: fix the javascript so that we don't use overlap image as im2t!!
    im2t_crop.write(overlap_image_fn);

    // high_resolution_clock::time_point before_diff = std::chrono::high_resolution_clock::now();
    // cout << "before diff image comp is: " << (duration_cast<duration<double>>(before_diff - after_ransac)).count() << flush;

    // difference image
    Magick::Image cdiff(im1_crop);
    get_diff_image(im1_crop, im2t_crop, cdiff);
    cdiff.write(diff_image_fn);

    // high_resolution_clock::time_point after_diff = std::chrono::high_resolution_clock::now();
    // cout << "after diff image comp is: " << (duration_cast<duration<double>>(after_diff - before_diff)).count() << flush;

    success = true;
    message = "";

  } catch( std::exception &e ) {
    success = false;
    std::ostringstream ss;
    ss << "Exception occured: [" << e.what() << "]";
    std::cout << ss.str() << std::endl;
    message = ss.str();
  }
}


///
/// Robust matching and Thin Plate Spline based image registration
///
/// Robust matching based on:
/// Tran, Q.H., Chin, T.J., Carneiro, G., Brown, M.S. and Suter, D., 2012, October. In defence of RANSAC for outlier rejection in deformable registration. ECCV.
///
/// Thin plate spline registration based on:
/// Bookstein, F.L., 1989. Principal warps: Thin-plate splines and the decomposition of deformations. IEEE TPAMI.
///
void imreg_sift::robust_ransac_tps(const char im1_fn[], const char im2_fn[],
                                   double xl, double xu, double yl, double yu,
                                   MatrixXd& Hopt, size_t& fp_match_count,
                                   const char im1_crop_fn[], const char im2_crop_fn[], const char im2_tx_fn[],
                                   const char diff_image_fn[],
                                   const char overlap_image_fn[],
                                   bool& success,
                                   std::string& message,
                                   imcomp_cache * cache,
                                   bool is_photometric) {
  try {
    auto start = std::chrono::high_resolution_clock::now();

    success = false;
    message = "";
    Magick::Image im1; im1.read( im1_fn );
    Magick::Image im2; im2.read( im2_fn );

    string im1_file_name(im1_fn);
    string im2_file_name(im2_fn);

    string base_filename1 = im1_file_name.substr(im1_file_name.find_last_of("/\\") + 1);
    std::string::size_type const p1(base_filename1.find_last_of('.'));
    base_filename1 = base_filename1.substr(0, p1);

    string base_filename2 = im2_file_name.substr(im2_file_name.find_last_of("/\\") + 1);
    std::string::size_type const p2(base_filename2.find_last_of('.'));
    base_filename2 = base_filename2.substr(0, p2);

    // to ensure that image pixel values are 8bit RGB
    im1.type(Magick::TrueColorType);
    im2.type(Magick::TrueColorType);

    // the photometric transformed image
    Magick::Image im2_pt(im2.size(), "black");
    im2_pt.type(Magick::TrueColorType);

    if (is_photometric) {
      compute_photo_transform(im1, im2, im2_pt);
    }

    Magick::Image im1_g = im1;
    im1_g.crop(Magick::Geometry(xu-xl, yu-yl, xl, yl));

    Magick::Image im2_g = im2;

    vector<VlSiftKeypoint> keypoint_list1, keypoint_list2;
    vector< vector<vl_uint8> > descriptor_list1, descriptor_list2;

    // check cache before computing features
    // TODO: Enable when we have a proper implementation of caching.
    // Disabled for now.
    // bool is_im1_cached = cache->get(base_filename1, keypoint_list1, descriptor_list1);
    // if (is_im1_cached) {
    //   cout << "using cached data for fid: " << base_filename1 << endl;
    // }
    // if (!is_im1_cached) {
    //   // images are converted to gray scale before processing
    //   compute_sift_features(im1_g, keypoint_list1, descriptor_list1, false);
    //   // cache for future use
    //   cache->put(base_filename1, keypoint_list1, descriptor_list1);
    //   cout << "successfully cached fid: " << base_filename1 << endl;
    // }
    // cout << "cached first keypoint is: " << keypoint_list1.at(1).x << " " << keypoint_list1.at(1).y << endl;

    // bool is_im2_cached = cache->get(base_filename2, keypoint_list2, descriptor_list2);
    // if (is_im2_cached) {
    //   cout << "using cached data for fid: " << base_filename2 << endl;
    // }
    // if (!is_im2_cached) {
    //   // images are converted to gray scale before processing
    //   compute_sift_features(im2_g, keypoint_list2, descriptor_list2, false);
    //   // cache for future use
    //   cache->put(base_filename2, keypoint_list2, descriptor_list2);
    //   cout << "successfully cached fid: " << base_filename2 << endl;
    // }
    // cout << "second keypoint is: " << keypoint_list2.at(1).x << " " << keypoint_list2.at(1).y << endl;

    // Compute SIFT features without caching.
    compute_sift_features(im1_g, keypoint_list1, descriptor_list1);
    compute_sift_features(im2_g, keypoint_list2, descriptor_list2);

    // use Lowe's 2nd nn test to find putative matches
    float threshold = 1.5f;
    std::vector< std::pair<uint32_t, uint32_t> > putative_matches;
    get_putative_matches(descriptor_list1, descriptor_list2, putative_matches, threshold);

    size_t n_lowe_match = putative_matches.size();
    //cout << "Putative matches (using Lowe's 2nd NN test) = " << putative_matches.size() << endl;

    if( n_lowe_match < 9 ) {
      fp_match_count = n_lowe_match;
      message = "Number of feature points that match are very low";
      return;
    }

    // Normalize points so that centroid lies at origin and mean distance to
    // original points in sqrt(2)
    //cout << "Creating matrix of size 3x" << n_lowe_match << " ..." << endl;
    MatrixXd pts1(3, n_lowe_match);
    MatrixXd pts2(3, n_lowe_match);
    //cout << "Creating point set matched using lowe ..." << endl;
    for( size_t i=0; i<n_lowe_match; ++i) {
      VlSiftKeypoint kp1 = keypoint_list1.at( putative_matches[i].first );
      VlSiftKeypoint kp2 = keypoint_list2.at( putative_matches[i].second );
      pts1(0, i) = kp1.x;
      pts1(1, i) = kp1.y;
      pts1(2, i) = 1.0;
      pts2(0, i) = kp2.x;
      pts2(1, i) = kp2.y;
      pts2(2, i) = 1.0;
    }
    //cout << "Normalizing points ..." << endl;

    Matrix3d pts1_tform;
    get_norm_matrix(pts1, pts1_tform);
    //cout << "Normalizing matrix (T) = " << pts1_tform << endl;

    MatrixXd pts1_norm = pts1_tform * pts1;
    MatrixXd pts2_norm = pts1_tform * pts2;

    // form a matrix containing match pair (x,y) <-> (x',y') as follows
    // [x y x' y'; ...]
    MatrixXd S_all(4, n_lowe_match);
    S_all.row(0) = pts1_norm.row(0);
    S_all.row(1) = pts1_norm.row(1);
    S_all.row(2) = pts2_norm.row(0);
    S_all.row(3) = pts2_norm.row(1);

    // initialize random number generator to randomly sample putative_matches
    mt19937 generator(9973);
    uniform_int_distribution<> dist(0, n_lowe_match-1);

    MatrixXd S(4,3), hatS(4,3);
    double residual;

    vector<size_t> best_robust_match_idx;
    // @todo: tune this threshold for better generalization
    double robust_ransac_threshold = 0.09;
    size_t RANSAC_ITER_COUNT = (size_t) ((double) n_lowe_match * 0.6);
    //cout << "RANSAC_ITER_COUNT = " << RANSAC_ITER_COUNT << endl;
    for( int i=0; i<RANSAC_ITER_COUNT; ++i ) {
      S.col(0) = S_all.col( dist(generator) ); // randomly select a match from S_all
      S.col(1) = S_all.col( dist(generator) );
      S.col(2) = S_all.col( dist(generator) );

      Vector4d muS = S.rowwise().mean();
      hatS = S.colwise() - muS;

      JacobiSVD<MatrixXd> svd(hatS, ComputeFullU);

      MatrixXd As = svd.matrixU().block(0, 0, svd.matrixU().rows(), 2);

      MatrixXd dx = S_all.colwise() - muS;
      MatrixXd del = dx - (As * As.transpose() * dx);

      vector<size_t> robust_match_idx;
      robust_match_idx.clear();
      for( int j=0; j<del.cols(); ++j ) {
        residual = sqrt( del(0,j)*del(0,j) + del(1,j)*del(1,j) + del(2,j)*del(2,j) + del(3,j)*del(3,j) );
        if ( residual < robust_ransac_threshold ) {
          robust_match_idx.push_back(j);
        }
      }

      //cout << i << ": robust_match_idx=" << robust_match_idx.size() << endl;
      if ( robust_match_idx.size() > best_robust_match_idx.size() ) {
        best_robust_match_idx.clear();
        best_robust_match_idx.swap(robust_match_idx);
        //cout << "[MIN]" << endl;
      }
    } // end RANSAC_ITER_COUNT loop

    fp_match_count = best_robust_match_idx.size();
    if ( fp_match_count < 3 ) {
      message = "Very low number of robust matching feature points";
      return;
    }
    //cout << "robust match pairs = " << fp_match_count << endl;

    // bin each correspondence pair into cells dividing the original image into KxK cells
    // a single point in each cell ensures that no two control points are very close
    unsigned int POINTS_PER_CELL = 1;
    unsigned int n_cell_w, n_cell_h;

    if( im1.rows() > 500 ) {
      n_cell_h = 9;
    } else {
      n_cell_h = 5;
    }
    unsigned int ch = (unsigned int) (im1.rows() / n_cell_h);

    if( im1.columns() > 500 ) {
      n_cell_w = 9;
    } else {
      n_cell_w = 5;
    }
    unsigned int cw = (unsigned int) (im1.columns() / n_cell_w);

    //printf("n_cell_w=%d, n_cell_h=%d, cw=%d, ch=%d", n_cell_w, n_cell_h, cw, ch);
    //printf("image size = %ld x %ld", im1.columns(), im1.rows());
    vector<size_t> sel_best_robust_match_idx;
    for( unsigned int i=0; i<n_cell_w; ++i ) {
      for( unsigned int j=0; j<n_cell_h; ++j ) {
        unsigned int xl = i * cw;
        unsigned int xh = (i+1)*cw - 1;
        if( xh > im1.columns() ) {
          xh = im1.columns() - 1;
        }

        unsigned int yl = j * ch;
        unsigned int yh = (j+1)*ch - 1;
        if( yh > im1.rows() ) {
          yh = im1.rows() - 1;
        }

        //printf("\ncell(%d,%d) = (%d,%d) to (%d,%d)", i, j, xl, yl, xh, yh);
        //cout << flush;

        vector< size_t > cell_pts;
        for( unsigned int k=0; k<best_robust_match_idx.size(); ++k ) {
          size_t match_idx = best_robust_match_idx.at(k);
          double x = pts1(0, match_idx);
          double y = pts1(1, match_idx);

          if( x >= xl && x < xh && y >= yl && y < yh ) {
            cell_pts.push_back( match_idx );
          }
        }
        if( cell_pts.size() >= POINTS_PER_CELL ) {
          uniform_int_distribution<> dist2(0, cell_pts.size()-1);
          for( unsigned int k=0; k<POINTS_PER_CELL; ++k ) {
            unsigned long cell_pts_idx = dist2(generator);
            sel_best_robust_match_idx.push_back( cell_pts.at(cell_pts_idx) );
          }
        }
      }
    }

    size_t n_cp = sel_best_robust_match_idx.size();
    MatrixXd cp1(2,n_cp);
    MatrixXd cp2(2,n_cp);

    for( size_t i=0; i<n_cp; ++i ) {
      unsigned long match_idx = sel_best_robust_match_idx.at(i);
      cp1(0,i) = pts1(0, match_idx ); cp1(1,i) = pts1(1, match_idx );
      cp2(0,i) = pts2(0, match_idx ); cp2(1,i) = pts2(1, match_idx );
    }

    // im1 crop
    Magick::Image im1_crop(im1);
    Magick::Geometry cropRect1(xu-xl, yu-yl, xl, yl);
    im1_crop.crop( cropRect1 );
    im1_crop.magick("JPEG");
    //cout << "\nWriting to " << im1_crop_fn << flush;
    im1_crop.write( im1_crop_fn );

    Magick::Image im2_crop(im2);
    im2_crop.crop( cropRect1 );
    //im2_crop.write( im2_crop_fn );

    double lambda = 0.001;
    double lambda_norm = lambda * (im1_crop.rows() * im1_crop.columns());

    // create matrix K
    MatrixXd K(n_cp, n_cp);
    K.setZero();
    double rx, ry, r, r2;
    for(unsigned int i=0; i<n_cp; ++i ) {
      for(unsigned int j=i; j<n_cp; ++j ) {
        // image grid coordinate = (i,j)
        // control point coordinate = cp(:,k)
        rx = cp1(0,i) - cp1(0,j);
        ry = cp1(1,i) - cp1(1,j);
        r  = sqrt(rx*rx + ry*ry);
        r2 = r * r;
        K(i, j) = r2*log(r2);
        K(j, i) = K(i,j);
      }
    }
    //cout << "K=" << K.rows() << "," << K.cols() << endl;
    //cout << "lambda = " << lambda << endl;
    // create matrix P
    MatrixXd P(n_cp, 3);
    for(unsigned int i=0; i<n_cp; ++i ) {
      //K(i,i) = 0; // ensure that the diagonal elements of K are 0 (as log(0) = nan)
      // approximating thin-plate splines based on:
      // Rohr, K., Stiehl, H.S., Sprengel, R., Buzug, T.M., Weese, J. and Kuhn, M.H., 2001. Landmark-based elastic registration using approximating thin-plate splines.
      K(i,i) = ((double) n_cp) * lambda * 1;
      P(i,0) = 1;
      P(i,1) = cp1(0,i);
      P(i,2) = cp1(1,i);
    }
    // cout << "P=" << P.rows() << "," << P.cols() << endl;
    //cout << "K=" << endl << K.block(0,0,6,6) << endl;
    // cout << "K=" << endl << K << endl;

    // create matrix L
    MatrixXd L(n_cp+3, n_cp+3);
    L.block(0, 0, n_cp, n_cp) = K;
    L.block(0, n_cp, n_cp, 3) = P;
    L.block(n_cp, 0, 3, n_cp) = P.transpose();
    L.block(n_cp, n_cp, 3, 3).setZero();
    // cout << "L rows and cols are: " << L.rows() << "," << L.cols() << endl;

    // create matrix V
    MatrixXd V(n_cp+3, 2);
    V.setZero();
    for(unsigned int i=0; i<n_cp; ++i ) {
      V(i,0) = cp2(0,i);
      V(i,1) = cp2(1,i);
    }
    MatrixXd Linv = L.inverse();
    MatrixXd W = Linv * V;

    // apply compyute transformations to second image
    // cout << "L matrix is: " << endl;
    // cout << L << endl;

    // Magick::Image im2t_crop( im1_crop.size(), "white");
    // string op_w_h_and_offsets;
    // Magick::DrawableAffine hinv_affine(W(0,0), W(1,1), W(1,0), W(0,1), 0, 0);
    // op_w_h_and_offsets = std::to_string(xu-xl) +
    //                      "x" +
    //                      std::to_string(yu-yl) +
    //                      "-" + to_string((int) W(0,2)) +
    //                      "-" + to_string((int) W(1,2));
    // im2t_crop.affineTransform(hinv_affine);
    // im2t_crop.artifact("distort:viewport", op_w_h_and_offsets);

    Magick::Image im2t_crop( im1_crop.size(), "white");
    double x0,x1,y0,y1;
    double x, y;
    double dx0, dx1, dy0, dy1;
    double fxy0, fxy1;
    double fxy_red, fxy_green, fxy_blue;
    double xi, yi;
    double x_non_linear_terms, y_non_linear_terms;
    double x_affine_terms, y_affine_terms;
    for(unsigned int j=0; j<im1_crop.rows(); j++) {
      for(unsigned int i=0; i<im1_crop.columns(); i++) {
        //cout << "(" << i << "," << j << ") :" << endl;
        xi = ((double) i) + 0.5; // center of pixel
        yi = ((double) j) + 0.5; // center of pixel
        x_non_linear_terms = 0.0;
        y_non_linear_terms = 0.0;
        for(unsigned int k=0; k<n_cp; ++k) {
          //cout << " k=" << k << endl;
          rx = cp1(0,k) - xi;
          ry = cp1(1,k) - yi;
          r = sqrt(rx*rx + ry*ry);
          r2 = r*r;
          x_non_linear_terms += W(k, 0) * r2 * log(r2);
          y_non_linear_terms += W(k, 1) * r2 * log(r2);
          //cout << "(" << x_non_linear_terms << "," << y_non_linear_terms << ")" << flush;
        }
        x_affine_terms = W(n_cp,0) + W(n_cp+1,0)*xi + W(n_cp+2,0)*yi;
        y_affine_terms = W(n_cp,1) + W(n_cp+1,1)*xi + W(n_cp+2,1)*yi;
        x = x_affine_terms + x_non_linear_terms;
        y = y_affine_terms + y_non_linear_terms;

        //printf("(%d,%d) : (xi,yi)=(%.2f,%.2f) (x,y)=(%.2f,%.2f) : (x,y)-affine = (%.2f,%.2f) (x,y)-nonlin = (%.2f,%.2f)", i, j, xi, yi, x, y, x_affine_terms, y_affine_terms, x_non_linear_terms, y_non_linear_terms);

        // neighbourhood of xh
        x0 = ((int) x);
        x1 = x0 + 1;
        dx0 = x - x0;
        dx1 = x1 - x;

        y0 = ((int) y);
        y1 = y0 + 1;
        dy0 = y - y0;
        dy1 = y1 - y;

        Magick::ColorRGB fx0y0, fx1y0, fx0y1, fx1y1;
        if (is_photometric) {
          fx0y0 = im2_pt.pixelColor(x0, y0);
          fx1y0 = im2_pt.pixelColor(x1, y0);
          fx0y1 = im2_pt.pixelColor(x0, y1);
          fx1y1 = im2_pt.pixelColor(x1, y1);
        } else {
          fx0y0 = im2.pixelColor(x0, y0);
          fx1y0 = im2.pixelColor(x1, y0);
          fx0y1 = im2.pixelColor(x0, y1);
          fx1y1 = im2.pixelColor(x1, y1);
        }

        // Bilinear interpolation: https://en.wikipedia.org/wiki/Bilinear_interpolation
        fxy0 = dx1 * fx0y0.red() + dx0 * fx1y0.red(); // note: x1 - x0 = 1
        fxy1 = dx1 * fx0y1.red() + dx0 * fx1y1.red(); // note: x1 - x0 = 1
        fxy_red = dy1 * fxy0 + dy0 * fxy1;

        fxy0 = dx1 * fx0y0.green() + dx0 * fx1y0.green(); // note: x1 - x0 = 1
        fxy1 = dx1 * fx0y1.green() + dx0 * fx1y1.green(); // note: x1 - x0 = 1
        fxy_green = dy1 * fxy0 + dy0 * fxy1;

        fxy0 = dx1 * fx0y0.blue() + dx0 * fx1y0.blue(); // note: x1 - x0 = 1
        fxy1 = dx1 * fx0y1.blue() + dx0 * fx1y1.blue(); // note: x1 - x0 = 1
        fxy_blue = dy1 * fxy0 + dy0 * fxy1;

        // cout << "red is: " << fx0y0.red() << " green is: " << fx0y0.green() << " blue is: " << fx0y0.blue() << endl;

        Magick::ColorRGB fxy(fxy_red, fxy_green, fxy_blue);
        im2t_crop.pixelColor(i, j, fxy);

      }
    }

    im2t_crop.write( im2_tx_fn );
    im2t_crop.write( overlap_image_fn );

    //auto finish = std::chrono::high_resolution_clock::now();
    //std::chrono::duration<double> elapsed = finish - start;
    //std::cout << "tps registration completed in " << elapsed.count() << " s" << endl;

    // difference image
    Magick::Image cdiff(im1_crop.size(), "black");
    get_diff_image(im1_crop, im2t_crop, cdiff);
    cdiff.write(diff_image_fn);

    success = true;
    message = "";

  } catch( std::exception &e ) {
    success = false;
    std::ostringstream ss;
    ss << "Exception occured: [" << e.what() << "]";
    message = ss.str();
  }
}
