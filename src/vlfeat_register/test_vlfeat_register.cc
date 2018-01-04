/*

Register an image pair based on SIFT features

Abhishek Dutta <adutta@robots.ox.ac.uk>
3 Jan. 2018

some code borrowed from: vlfeat-0.9.20/src/sift.c

*/

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
#include <fstream>

#include <Eigen/Dense>
#include <Eigen/SVD>

using namespace Eigen;

using namespace std;

bool compute_sift_features(const string filename, vector<VlSiftKeypoint>& keypoint_list, vector< vector<vl_uint8> >& descriptor_list, bool verbose=false) {
  vl_bool  err    = VL_ERR_OK ;
  char     err_msg [1024] ;

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

  FILE            *in    = 0 ;
  vl_uint8        *data  = 0 ;
  vl_sift_pix     *fdata = 0 ;
  VlPgmImage       pim ;
  vl_size          q ;
  int              i ;
  vl_bool          first ;

  double           *ikeys = 0 ;
  int              nikeys = 0, ikeys_size = 0 ;

  // load pgm image
  const char* name = filename.c_str();
  in = fopen (filename.c_str(), "rb") ;
  if (!in) {
    err = VL_ERR_IO ;
    snprintf(err_msg, sizeof(err_msg),
             "Could not open '%s' for reading.", name) ;
    goto done ;
  }

  err = vl_pgm_extract_head (in, &pim) ;
  if (err) {
    switch (vl_get_last_error()) {
    case  VL_ERR_PGM_IO :
      snprintf(err_msg, sizeof(err_msg),
               "Cannot read from '%s'.", name) ;
      err = VL_ERR_IO ;
      break ;

    case VL_ERR_PGM_INV_HEAD :
      snprintf(err_msg, sizeof(err_msg),
               "'%s' contains a malformed PGM header.", name) ;
      err = VL_ERR_IO ;
      goto done ;
    }
  }

  /* allocate buffer */
  data  = malloc(vl_pgm_get_npixels (&pim) *
                 vl_pgm_get_bpp       (&pim) * sizeof (vl_uint8)   ) ;
  fdata = malloc(vl_pgm_get_npixels (&pim) *
                 vl_pgm_get_bpp       (&pim) * sizeof (vl_sift_pix)) ;

  if (!data || !fdata) {
    err = VL_ERR_ALLOC ;
    snprintf(err_msg, sizeof(err_msg),
             "Could not allocate enough memory.") ;
    goto done ;
  }

  /* read PGM body */
  err  = vl_pgm_extract_data (in, &pim, data) ;

  if (err) {
    snprintf(err_msg, sizeof(err_msg), "PGM body malformed.") ;
    err = VL_ERR_IO ;
    goto done ;
  }

  /* convert data type */
  for (q = 0 ; q < (unsigned) (pim.width * pim.height) ; ++q) {
    fdata [q] = data [q] ;
  }

  // create filter
  VlSiftFilt *filt = 0 ;
  filt = vl_sift_new (pim.width, pim.height, O, S, o_min) ;

  if (peak_thresh >= 0) vl_sift_set_peak_thresh (filt, peak_thresh) ;
  if (edge_thresh >= 0) vl_sift_set_edge_thresh (filt, edge_thresh) ;
  if (norm_thresh >= 0) vl_sift_set_norm_thresh (filt, norm_thresh) ;
  if (magnif      >= 0) vl_sift_set_magnif      (filt, magnif) ;
  if (window_size >= 0) vl_sift_set_window_size (filt, window_size) ;

  if (!filt) {
    snprintf (err_msg, sizeof(err_msg),
              "Could not create SIFT filter.") ;
    err = VL_ERR_ALLOC ;
    goto done ;
  }

  if(verbose) {
    cout << "\nvl_sift: filter settings:" << flush;
    cout << "\nvl_sift:   octaves      (O)      = " << vl_sift_get_noctaves(filt);
    cout << "\nvl_sift:   levels       (S)      = " << vl_sift_get_nlevels(filt);
    cout << "\nvl_sift:   first octave (o_min)  = " << vl_sift_get_octave_first(filt);
    cout << "\nvl_sift:   edge thresh           = " << vl_sift_get_edge_thresh(filt);
    cout << "\nvl_sift:   peak thresh           = " << vl_sift_get_peak_thresh(filt);
    cout << "\nvl_sift:   norm thresh           = " << vl_sift_get_norm_thresh(filt);
    cout << "\nvl_sift:   window size           = " << vl_sift_get_window_size(filt);
  }

  /* ...............................................................
   *                                             Process each octave
   * ............................................................ */
  i     = 0 ;
  first = 1 ;
  descriptor_list.clear();
  keypoint_list.clear();
  while (1) {
    if(verbose) {
      cout << "\n**** processing octave " << vl_sift_get_octave_index (filt) << flush;
    }

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

    if(verbose) {
      cout << "\n\tvl_sift: GSS octave " << vl_sift_get_octave_index (filt) << " computed" << flush;
    }

    /* run detector ............................................. */
    vl_sift_detect(filt) ;
    keys  = vl_sift_get_keypoints(filt) ;
    nkeys = vl_sift_get_nkeypoints(filt) ;
    i     = 0 ;

    if(verbose) {
      cout << "\n\tvl_sift: detected " << nkeys << " (unoriented) keypoints" << flush;
    }

    /* for each keypoint ........................................ */
    for (; i < nkeys ; ++i) {
      double                angles [4] ;
      int                   nangles ;
      VlSiftKeypoint const *k ;

      /* obtain keypoint orientations ........................... */
      k = keys + i ;

      VlSiftKeypoint key_data;
      key_data.x = k->x;
      key_data.y = k->y;
      key_data.sigma = k->sigma;

      nangles = vl_sift_calc_keypoint_orientations(filt, angles, k) ;
      //cout << "\nnangles=" << nangles << ", angles = " << angles[0] << ", " << angles[1] << ", " << angles[2] << ", " << angles[3] << flush;

      /* for each orientation ................................... */
      for (q = 0 ; q < (unsigned) nangles ; ++q) {
        vl_sift_pix descr[128];

        /* compute descriptor (if necessary) */
        vl_sift_calc_keypoint_descriptor(filt, descr, k, angles [q]) ;

        vector<vl_uint8> descriptor(128, 255);
        for( size_t l=0; l<128; l++ ) {
          double value = descr[l] * 512.0;
          if(value < 255.0) {
            descriptor[l] = (vl_uint8) (value);
          }
          //cout << "[" << value << ":" << (int) descriptor[l] << "], ";
        }
        descriptor_list.push_back(descriptor);
        ++ndescriptors;

        keypoint_list.push_back(key_data); // add corresponding keypoint
      }
    }
  }
  if(verbose) {
    cout << "\n\tvl_sift: found " << ndescriptors << " keypoints" << flush;
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

    /* release image data */
    if (data) {
      free (data) ;
      data = 0 ;
    }

    /* close files */
    if (in) {
      fclose (in) ;
      in = 0 ;
    }

    /* if bad print error message */
    if (err) {
      fprintf
        (stderr,
         "sift: err: %s (%d)\n",
         err_msg,
         err) ;
      return false;
    } else {
      return true;
    }
}

inline float dist_l2(const vector<float> a, const vector<float> b, unsigned int ndim)
{
  float ret = 0.0f;
  for (unsigned d=0; d<ndim; ++d) {
    ret += (a[d] - b[d])*(a[d] - b[d]);
  }
  return ret;
}

void get_putative_matches(vector< vector<vl_uint8> >& descriptor_list1, vector< vector<vl_uint8> >& descriptor_list2, std::vector< std::pair<uint32_t, uint32_t> > &putative_matches, float threshold) {

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

int main (int argc, const char * argv[]) {
  cout << "Image Registration using vlfeat\n" << flush;
  if(argc!=3) {
    cout << "  Usage: " << argv[0] << " image1_filename image2_filename" << endl << flush;
    return 0;
  }

  string filename1 = argv[1];
  string filename2 = argv[2];

  vector<VlSiftKeypoint> keypoint_list1, keypoint_list2;
  vector< vector<vl_uint8> > descriptor_list1, descriptor_list2;

  bool result1 = compute_sift_features(filename1, keypoint_list1, descriptor_list1, true);
  cout << "\nFilename = " << filename1 << ", result=" << result1 << flush;
  cout << "\n  keypoint_list = " << keypoint_list1.size() << flush;
  cout << "\n  descriptor_list = " << descriptor_list1.size() << flush;

  ofstream kp("/home/tlm/kp1.txt");
  for( size_t i=0; i<keypoint_list1.size(); i++ ) {
    kp << keypoint_list1[i].x << "," << keypoint_list1[i].y << "," << keypoint_list1[i].sigma << endl;
  }
  kp.close();

/*
  bool result2 = compute_sift_features(filename2, keypoint_list2, descriptor_list2);
  cout << "\nFilename = " << filename2 << ", result=" << result2 << flush;
  cout << "\n  keypoint_list = " << keypoint_list2.size() << flush;
  cout << "\n  descriptor_list = " << descriptor_list2.size() << flush;
*/
/*
  for( size_t i=10; i<14; i++ ) {
    cout << "\ndescriptor_list1[" << i << "]: " << flush;
    for( size_t j=0; j<descriptor_list1[i].size(); j++ ) {
      cout << (int) descriptor_list1[i][j] << ", ";
    }
  }
  cout << flush;
*/

/*
  // use Lowe's 2nd nn test to find putative matches
  float threshold = 1.5f;
  std::vector< std::pair<uint32_t, uint32_t> > putative_matches;
  get_putative_matches(descriptor_list1, descriptor_list2, putative_matches, threshold);

  cout << "\nShowing putative matches :" << flush;
  for( size_t i=0; i<putative_matches.size(); i++ ) {
    cout << "[" << putative_matches[i].first << ":" << putative_matches[i].second << "], " << flush;
  }
  cout << "\nPutative matches (using Lowe's 2nd NN test) = " << putative_matches.size() << flush;
*/

/*
  // initialize random number generator to randomly sample putative_matches
  size_t n_match = putative_matches.size();
  random_device rand_device;
  mt19937 generator(rand_device());
  uniform_int_distribution<> dist(0, n_match-1);

  // estimate homography using RANSAC
  size_t max_score = 0;
  Matrix3d max_score_H(3,3);

  for( unsigned int iter=0; iter<10000; iter++ ) {
    //cout << "\niter=" << iter << flush;

    // randomly select 4 matches from putative_matches
    int pm1 = dist(generator);
    int pm2 = dist(generator);
    int pm3 = dist(generator);
    int pm4 = dist(generator);
    //cout << "\nRandom entries from putative_matches: " << pm1 << "," << pm2 << "," << pm3 << "," << pm4 << flush;

    MatrixXd A(12,9);
    A.setZero();

    unsigned int pmi_list[4];
    pmi_list[0] = dist(generator);
    pmi_list[1] = dist(generator);
    pmi_list[2] = dist(generator);
    pmi_list[3] = dist(generator);
    //cout << "\n  forming matrix A" << flush;
    for( unsigned int i=0; i<4; i++ ) {
      unsigned int pmi = pmi_list[i];
      VlSiftKeypoint kp1 = keypoint_list1[putative_matches[pmi].first];
      VlSiftKeypoint kp2 = keypoint_list2[putative_matches[pmi].second];
      Vector3d x1( kp1.x, kp1.y, 1.0 );
      Vector3d x2( kp2.x, kp2.y, 1.0 );

      for( unsigned int j=0; j<3; j++ ) {
        A(i*3 + 0, j*3 + 1) = -x1(j) * x2(2);
        A(i*3 + 0, j*3 + 2) =  x1(j) * x2(1);
        A(i*3 + 1, j*3 + 0) =  x1(j) * x2(2);
        A(i*3 + 1, j*3 + 2) = -x1(j) * x2(0);
        A(i*3 + 2, j*3 + 0) = -x1(j) * x2(1);
        A(i*3 + 2, j*3 + 1) =  x1(j) * x2(0);
      }
    }
    JacobiSVD<MatrixXd> svd(A, ComputeThinV);
    MatrixXd V = svd.matrixV().col(8);
    MatrixXd H(V);
    H.resize(3,3);
    //cout << "\n  H=" << H << flush;

    size_t score = 0;
    for( unsigned int k=0; k<putative_matches.size(); k++ ) {
      //cout << "\n  k=" << k << flush;
      VlSiftKeypoint kp1_handle = keypoint_list1[putative_matches[k].first];
      VlSiftKeypoint kp2_handle = keypoint_list2[putative_matches[k].second];
      Vector3d kp1( kp1_handle.x, kp1_handle.y, 1.0 );
      Vector3d kp2( kp2_handle.x, kp2_handle.y, 1.0 );
      //cout << "| kp1=" << kp1(0) << "," << kp1(1) << "," << kp1(2) << flush;
      //cout << "| kp2=" << kp2(0) << "," << kp2(1) << "," << kp2(2) << flush;

      MatrixXd kp2_comp = H * kp1;
      //cout << "| kp2_comp=" << kp2_comp(0) << "," << kp2_comp(1) << "," << kp2_comp(2) << flush;
      double dx = (kp2_comp(0) / kp2_comp(2)) - (kp2(0) / kp2(2));
      double dy = (kp2_comp(1) / kp2_comp(2)) - (kp2(1) / kp2(2));
      //cout << "| dx=" << dx << ", dy=" << dy << flush;
      if ( (dx*dx + dy*dy) < 9 ) { // tolerate 3 pixel error in x and y
        score++;
      }
    }
    if( score > max_score ) {
      max_score = score;
      max_score_H = H;
    }
    MatrixXd Hdisp(H);
    Hdisp.resize(1,9);
    //cout << "\niter " << iter << " : H=[" << Hdisp << "], score=" << score << ", max_score=" << max_score << ", total_matches=" << putative_matches.size() << flush;
  }
  MatrixXd max_score_H_disp(max_score_H);
  max_score_H_disp.resize(1,9);
  cout << "\nResult : H=[" << max_score_H_disp << "], score=" << max_score << ", total_matches=" << putative_matches.size() << flush;

*/

  cout << "\nDone\n" << flush;
  return 0;
}
