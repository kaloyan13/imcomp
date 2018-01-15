/*

Register an image pair based on SIFT features

Abhishek Dutta <adutta@robots.ox.ac.uk>
3 Jan. 2018

some code borrowed from: vlfeat-0.9.20/src/sift.c

*/

#include "vl_register_images.h"

void vl_register_images::compute_sift_features(const string filename, vector<VlSiftKeypoint>& keypoint_list, vector< vector<vl_uint8> >& descriptor_list, bool verbose) {
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

  // create filter
  VlSiftFilt *filt = 0 ;

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

  if(verbose) {
    cout << "\nimage dimension = " << pim.width << " x " << pim.height << flush;
    cout << "\nimage data = " << flush;
    for (q = 0 ; q < 10 ; ++q) {
      fdata [q] = data [q] ;
      cout << (int) data[q] << ", " << flush;
    }
  }

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
      cout << "\n\n**** processing octave " << vl_sift_get_octave_index (filt) << flush;
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
      cout << "\n\tvl_sift: detected " << nkeys << " (unoriented) keypoints\n" << flush;
    }

    for (i=0; i < nkeys ; ++i) {
      VlSiftKeypoint const *k ;
      k = keys+i;
      //printf ("keypoint[%d] : (x,y,sigma)=(%.2f,%.2f,%.5f)\n", i, k->x, k->y, k->sigma) ;
      //printf ("%.2f,%.2f\n", k->x, k->y) ;
      //printf ("%d,%d\n", (int) k->x, (int) k->y) ;
    }
    i = 0;

    /* for each keypoint ........................................ */
    for (; i < nkeys ; ++i) {
      double                angles [4] ;
      int                   nangles ;
      VlSiftKeypoint const *k ;

      /* obtain keypoint orientations ........................... */
      k = keys + i ;

      VlSiftKeypoint key_data = *k;

      nangles = vl_sift_calc_keypoint_orientations(filt, angles, k) ;
      //cout << "\nnangles=" << nangles << ", k->s=" << (k->s) << ", k->o=" << (k->o) << ", vl_sift_get_octave_index (filt)=" << vl_sift_get_octave_index (filt) << endl<<flush;

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

        if(verbose) {
          printf ("  - k(%.2f,%.2f) angle[%d]=%.2f | descriptor=[%d,%d,%d,%d,%d, ..., %d,%d,%d,%d]\n",k->x, k->y, (int)q, angles[q], d[0], d[1], d[2], d[3], d[4], d[124], d[125], d[126], d[127]) ;
        }
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
    }
}

void vl_register_images::get_putative_matches(vector< vector<vl_uint8> >& descriptor_list1, vector< vector<vl_uint8> >& descriptor_list2, std::vector< std::pair<uint32_t, uint32_t> > &putative_matches, float threshold) {
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

void vl_register_images::register_images(const char fullSizeFn1[], const char fullSizeFn2[],
                                double xl, double xu, double yl, double yu,
                                homography &Hinit, uint32_t& bestNInliners,
                                const char outFn1[], const char outFn2[], const char outFn2t[],
                                const char diff_image[],
                                const char overlap_image[]) {

  Magick::Image im1; im1.read( fullSizeFn1 );
  Magick::Image im2; im2.read( fullSizeFn2 );

  // temp image file where transformed image is written in each iteration
  boost::filesystem::path tmp_dir = boost::filesystem::temp_directory_path() / "imcomp";
  tmp_dir = tmp_dir / "tmp";
  if( !boost::filesystem::exists(tmp_dir) ) {
    boost::filesystem::create_directories(tmp_dir);
  }

  boost::filesystem::path filename1 = tmp_dir / boost::filesystem::unique_path("file1_grayscale_%%%%-%%%%-%%%%-%%%%.pgm");
  boost::filesystem::path filename2 = tmp_dir / boost::filesystem::unique_path("file2_grayscale_%%%%-%%%%-%%%%-%%%%.pgm");
  Magick::Image im1_g = im1;
  im1_g.magick("pgm");
  im1_g.crop( Magick::Geometry(xu-xl, yu-yl, xl, yl) );
  im1_g.write(filename1.string().c_str());
  Magick::Image im2_g = im2;
  im2_g.magick("pgm");
  im2_g.write(filename2.string().c_str());

/**/
  vector<VlSiftKeypoint> keypoint_list1, keypoint_list2;
  vector< vector<vl_uint8> > descriptor_list1, descriptor_list2;

  compute_sift_features(filename1.string().c_str(), keypoint_list1, descriptor_list1);
  cout << "\nFilename = " << filename1 << flush;
  cout << "\n  keypoint_list = " << keypoint_list1.size() << flush;
  cout << "\n  descriptor_list = " << descriptor_list1.size() << flush;


  compute_sift_features(filename2.string().c_str(), keypoint_list2, descriptor_list2);
  cout << "\nFilename = " << filename2 << flush;
  cout << "\n  keypoint_list = " << keypoint_list2.size() << flush;
  cout << "\n  descriptor_list = " << descriptor_list2.size() << flush;

  // use Lowe's 2nd nn test to find putative matches
  float threshold = 1.5f;
  std::vector< std::pair<uint32_t, uint32_t> > putative_matches;
  get_putative_matches(descriptor_list1, descriptor_list2, putative_matches, threshold);

/*
  cout << "\nShowing putative matches :" << flush;
  for( size_t i=0; i<putative_matches.size(); i++ ) {
    cout << "[" << putative_matches[i].first << ":" << putative_matches[i].second << "], " << flush;
  }
*/
  cout << "\nPutative matches (using Lowe's 2nd NN test) = " << putative_matches.size() << flush;


/**/
  // initialize random number generator to randomly sample putative_matches
  size_t n_match = putative_matches.size();
  random_device rand_device;
  mt19937 generator(rand_device());
  uniform_int_distribution<> dist(0, n_match-1);

  // estimate homography using RANSAC
  size_t max_score = 0;
  Matrix3d max_score_H(3,3);
  MatrixXd best_singular_values;
  MatrixXd bestV;
  MatrixXd bestA;

  for( unsigned int iter=0; iter<1000; iter++ ) {
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
    JacobiSVD<MatrixXd> svd(A, ComputeFullV);

    MatrixXd V = svd.matrixV().col(8);
    MatrixXd H(V);
    H.resize(3,3);

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
      best_singular_values = svd.singularValues();
      bestV = svd.matrixV();
      bestA = A;
    }
    /*
    MatrixXd Hdisp(H);
    Hdisp.resize(1,9);
    cout << "\niter " << iter << " : H=[" << Hdisp << "], score=" << score << ", max_score=" << max_score << ", total_matches=" << putative_matches.size() << flush;
    */
  }
/*
  MatrixXd max_score_H_disp(max_score_H);
  max_score_H_disp.resize(1,9);
  cout << "\nResult : H=[" << max_score_H_disp << "], score=" << max_score << ", total_matches=" << putative_matches.size() << flush;

  cout << "\n  inliers=" << max_score << flush;
  cout << "\n  bestH=\n" << max_score_H << flush;
  cout << "\n  bestA=\n" << bestA << flush;
  cout << "\n  best_singular_values=\n" << best_singular_values << flush;
  cout << "\n  bestV=\n" << bestV << flush;
*/
  max_score_H = max_score_H / max_score_H(2,2);
  cout << "\nmax_score_H (norm) = \n" << max_score_H << flush;

  // im1 crop
  Magick::Image im1_crop(im1);
  Magick::Geometry cropRect1(xu-xl, yu-yl, xl, yl);
  im1_crop.crop( cropRect1 );
  im1_crop.write( outFn1 );

  // im2 crop and transform
  MatrixXd H = max_score_H;
  // used by caller of register_images()
  Hinit.H[0] = H(0,0); Hinit.H[1] = H(0,1); Hinit.H[2] = H(0,2);
  Hinit.H[3] = H(1,0); Hinit.H[4] = H(1,1); Hinit.H[5] = H(1,2);
  Hinit.H[6] = H(2,0); Hinit.H[7] = H(2,1); Hinit.H[8] = H(2,2);

  Magick::Image im2t_crop( im1_crop.size(), "white");

  double x0,x1,y0,y1;
  double x, y, homogeneous_norm;
  double dx0, dx1, dy0, dy1;
  double fxy0, fxy1;
  double fxy_red, fxy_green, fxy_blue;
  double xi, yi;
  Magick::Image diff1(im1_crop.size(), "black");
  Magick::Image diff2(im1_crop.size(), "black");
  Magick::Image diff(im1_crop.size(), "white");
  Magick::Image overlap(im1_crop.size(), "white");

  cout << "\nComputing transformed image ..." << flush;
  for(unsigned int j=0; j<im2t_crop.rows(); j++) {
    for(unsigned int i=0; i<im2t_crop.columns(); i++) {
      xi = ((double) i) + 0.5; // center of pixel
      yi = ((double) j) + 0.5; // center of pixel
      x = H(0,0) * xi + H(0,1) * yi + H(0,2);
      y = H(1,0) * xi + H(1,1) * yi + H(1,2);
      homogeneous_norm = H(2,0) * xi + H(2,1) * yi + H(2,2);
      x = x / homogeneous_norm;
      y = y / homogeneous_norm;

      // neighbourhood of xh
      x0 = ((int) x);
      x1 = x0 + 1;
      dx0 = x - x0;
      dx1 = x1 - x;

      y0 = ((int) y);
      y1 = y0 + 1;
      dy0 = y - y0;
      dy1 = y1 - y;

      Magick::ColorRGB fx0y0 = im2.pixelColor(x0, y0);
      Magick::ColorRGB fx1y0 = im2.pixelColor(x1, y0);
      Magick::ColorRGB fx0y1 = im2.pixelColor(x0, y1);
      Magick::ColorRGB fx1y1 = im2.pixelColor(x1, y1);

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

      Magick::ColorRGB fxy(fxy_red, fxy_green, fxy_blue);
      im2t_crop.pixelColor(i, j, fxy);

      // compute difference image
      Magick::ColorRGB c1 = im1_crop.pixelColor(i,j);
      double avg1 = ((double)(c1.red() + c1.green() + c1.blue())) / (3.0f);
      double avg2 = (fxy_red + fxy_green + fxy_blue) / (3.0f);
      double diff_val1 = avg1 - avg2;
      double diff_val2 = avg2 - avg1;

      if( diff_val1 > 0.3 ) {
        diff.pixelColor(i, j, Magick::ColorRGB(0, 0.447, 0.698)); // blue color safe for the color blind 
      }
      if( diff_val2 > 0.3 ) {
        diff.pixelColor(i, j, Magick::ColorRGB(0.835, 0.368, 0)); // blue color safe for the color blind 
      }

      // overlap
      double red_avg = (c1.red() + fxy_red) / (2.0f);
      double green_avg = (c1.green() + fxy_green) / (2.0f);
      double blue_avg = (c1.blue() + fxy_blue) / (2.0f);
      overlap.pixelColor(i, j, Magick::ColorRGB(red_avg, green_avg, blue_avg));
    }
  }
  im2t_crop.write( outFn2t );

  // difference image
  diff.write(diff_image);
  overlap.write(overlap_image);

  cout << "\nWritten transformed images.\n" << flush;
}

