/*

Register an image pair based on SIFT features

Abhishek Dutta <adutta@robots.ox.ac.uk>
3 Jan. 2018

some code borrowed from: vlfeat-0.9.20/src/sift.c

*/

#include "vl_register_images.h"

void vl_register_images::compute_sift_features(const string filename, vector<VlSiftKeypoint>& keypoint_list, vector< vector<vl_uint8> >& descriptor_list, bool verbose=false) {
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
  static const double expandOutBy= 0.1;

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

  // cleanup
  cout << "\nclearning up tmp files " << filename1 << flush;
  //boost::filesystem::remove(filename1);
  cout << "\nclearning up tmp files " << filename2 << flush;
  //boost::filesystem::remove(filename2);

  // use Lowe's 2nd nn test to find putative matches
  cout << "\ncomputing putative matches " << flush;
  float threshold = 1.5f;
  std::vector< std::pair<uint32_t, uint32_t> > putative_matches;
  get_putative_matches(descriptor_list1, descriptor_list2, putative_matches, threshold);
  cout << "\nPutative matches (using Lowe's 2nd NN test) = " << putative_matches.size() << flush;

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

  homography H;
  H.H[0] = max_score_H(0,0); H.H[1] = max_score_H(0,1); H.H[2] = max_score_H(0,2);
  H.H[3] = max_score_H(1,0); H.H[4] = max_score_H(1,1); H.H[5] = max_score_H(1,2);
  H.H[6] = max_score_H(2,0); H.H[7] = max_score_H(2,1); H.H[8] = max_score_H(2,2);
  H.normLast();

  // draw resulting images
  double xl_= xl, xu_= xu, yl_= yl, yu_= yu;
  double dw_= expandOutBy*(xu-xl), dh_= expandOutBy*(yu-yl);
  xl_= std::max(0.0, xl_-dw_/2);
  yl_= std::max(0.0, yl_-dh_/2);
  xu_= std::min(static_cast<double>(im1.columns() ), xu_+dw_/2);
  yu_= std::min(static_cast<double>(im1.rows()), yu_+dh_/2);
  Magick::Geometry cropRect1(xu_-xl_, yu_-yl_, xl_, yl_);
  double xl2_, xu2_, yl2_, yu2_;
  findBBox2( xl_, xu_, yl_, yu_, H, xl2_, xu2_, yl2_, yu2_, im2.columns(), im2.rows() );
  Magick::Geometry cropRect2(xu2_-xl2_, yu2_-yl2_, xl2_, yl2_);

  im1.crop( cropRect1 );
  im1.write( outFn1 );
  im2.crop( cropRect2 );
  im2.write( outFn2 );

  Magick::Image im2t = im2;
  double Hinv[9];
  H.getInverse(Hinv);
  homography::normLast(Hinv);
  // AffineProjection(sx, rx, ry, sy, tx, ty) <=> H=[sx, ry, tx; sy, rx, ty; 0 0 1]
  double MagickAffine[6] = {Hinv[0], Hinv[3], Hinv[1], Hinv[4], Hinv[2], Hinv[5]};
  im2t.virtualPixelMethod(Magick::BlackVirtualPixelMethod);
  if ( im1.rows() == im2.rows() || im1.columns() == im2.columns() ) {
    im2t.distort(Magick::AffineProjectionDistortion, 6, MagickAffine, false);
  } else {
    im2t.distort(Magick::AffineProjectionDistortion, 6, MagickAffine, true);
  }
  //std::cout << "\nim2t = " << im2t.columns() << "," << im2t.rows() << std::flush;
  im2t.crop( cropRect1 );
  //std::cout << "\ncropRect1=" << cropRect1.width() << "," << cropRect1.height() << "," << cropRect1.xOff() << "," << cropRect1.yOff() << std::flush;
  //std::cout << "\nim2t (cropped) = " << im2t.columns() << "," << im2t.rows() << std::flush;
  im2t.write( outFn2t );

  Hinit = H;

  // create difference image
  // red channel  : image 1
  // green channel: image 2
  // blue         : min(image1, image2)
  // see: https://stackoverflow.com/a/33673440/7814484
  Magick::Image diff( im1.size(), "white");
  Magick::Image overlap( im1.size(), "white");
  for(unsigned int x=0; x<im1.columns(); x++) {
    for(unsigned int y=0; y<im1.rows(); y++) {
      Magick::Color c1 = im1.pixelColor(x,y);
      Magick::Color c2 = im2t.pixelColor(x,y);

      unsigned int c1_avg = ( c1.redQuantum() + c1.greenQuantum() + c1.blueQuantum() ) / 3;
      unsigned int c2_avg = ( c2.redQuantum() + c2.greenQuantum() + c2.blueQuantum() ) / 3;

      unsigned dark = c1_avg;
      if ( c2_avg < c1_avg ) {
        dark = c2_avg;
      }
      Magick::Color cdiff = Magick::Color(c1_avg, c2_avg, dark);
      diff.pixelColor(x, y, cdiff);

      unsigned int overlay_r = ( c1.redQuantum() + c2.redQuantum() ) / 2;
      unsigned int overlay_g = ( c1.greenQuantum() + c2.greenQuantum() ) / 2;
      unsigned int overlay_b = ( c1.blueQuantum() + c2.blueQuantum() ) / 2;

      overlap.pixelColor(x, y, Magick::Color(overlay_r, overlay_g, overlay_b));
    }
  }
  diff.write(diff_image);
  overlap.write(overlap_image);

  cout << "\nDone\n" << flush;
  return 0;
}

void vl_register_images::findBBox2( double xl, double xu, double yl, double yu, homography const &H, double &xl2, double &xu2, double &yl2, double &yu2, uint32_t w2, uint32_t h2 ){
  if( fabs(H.H[8]-1.0) > 1e-5 ) {
    cerr << "vl_register_images::findBBox2() : malformed Homography matrix\n" << flush;
  }

  xl2= 10000; xu2= -10000; yl2= 10000; yu2= -10000;
  double x_, y_;

  homography::affTransform(H.H, xl, yl, x_, y_);
  xl2= std::min(xl2,x_); xu2= std::max(xu2,x_); yl2= std::min(yl2,y_); yu2= std::max(yu2,y_);

  homography::affTransform(H.H, xl, yu, x_, y_);
  xl2= std::min(xl2,x_); xu2= std::max(xu2,x_); yl2= std::min(yl2,y_); yu2= std::max(yu2,y_);

  homography::affTransform(H.H, xu, yl, x_, y_);
  xl2= std::min(xl2,x_); xu2= std::max(xu2,x_); yl2= std::min(yl2,y_); yu2= std::max(yu2,y_);

  homography::affTransform(H.H, xu, yu, x_, y_);
  xl2= std::min(xl2,x_); xu2= std::max(xu2,x_); yl2= std::min(yl2,y_); yu2= std::max(yu2,y_);

  xl2= std::max(0.0,xl2);
  yl2= std::max(0.0,yl2);
  xu2= std::min(xu2,static_cast<double>(w2));
  yu2= std::min(yu2,static_cast<double>(h2));
}
