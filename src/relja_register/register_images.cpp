/*
==== Author:

Relja Arandjelovic (relja@robots.ox.ac.uk)
Visual Geometry Group,
Department of Engineering Science
University of Oxford

==== Copyright:

The library belongs to Relja Arandjelovic and the University of Oxford.
No usage or redistribution is allowed without explicit permission.

Notes:

Feb. 13, 2017 : Abhishek Dutta <adutta@robots.ox.ac.uk>
 - Added compute_homography() for applications that just require the affine
   transform matrix (and not the blended or transformed images)

Oct. 10, 2017 : Abhishek Dutta <adutta@robots.ox.ac.uk>
 - Added get_ransac_inliers() for methods that need to be seeded with a set of
   initial matching points
*/

#include "register_images.h"

#include <algorithm>
#include <math.h>
#include <vector>

#include <boost/thread.hpp>
#include <boost/filesystem.hpp>

#include <Magick++.h>

#include "ellipse.h"
#include "feat_getter.h"
#include "feat_standard.h"
#include "putative.h"
#include "det_ransac.h"
#include "macros.h"



//---- stuff for parallel feature extraction

class featWorker {

    public:

        featWorker( featGetter *aFeatGetterObj, const char aFn[],
		double aXl, double aXu, double aYl, double aYu,
		uint32_t &aNumFeats, std::vector<ellipse> &aRegions, float **&aDescs ) : featGetterObj(aFeatGetterObj), fn(aFn), xl(aXl), xu(aXu), yl(aYl), yu(aYu), numFeats(&aNumFeats), regions(&aRegions), descs(&aDescs)
            {}

    void
        operator()(){
            featGetterObj->getFeats( fn,
                            static_cast<uint32_t>(xl), static_cast<uint32_t>(xu),
                            static_cast<uint32_t>(yl), static_cast<uint32_t>(yu),
                            *numFeats, *regions, *descs );
        }

    private:
        featGetter *featGetterObj;
        const char *fn;
        double xl, xu, yl, yu;
        uint32_t *numFeats;
        std::vector<ellipse> *regions;
        float ***descs;

};

void
registerImages::registerFromGuess(
                                  const char image_fn1[], const char image_fn2[],
                                  double xl, double xu, double yl, double yu,
                                  homography &Hinit, uint32_t& bestNInliers,
                                  const char outFn1[], const char outFn2[], const char outFn2t[],
                                  const char diff_image[],
                                  const char *fullSizeFn1, const char *fullSizeFn2 ) {

  static const double expandOutBy= 0.1;
  featGetter *featGetterObj= new featGetter_standard( "hesaff-rootsift" );

  bool fullSizeExist= false;
  if (fullSizeFn1!=NULL || fullSizeFn2!=NULL)
    fullSizeExist= true;
  if (fullSizeFn1==NULL) fullSizeFn1= image_fn1;
  if (fullSizeFn2==NULL) fullSizeFn2= image_fn2;

  //std::cout << "\nfullSizeFn1 = " << fullSizeFn1 << std::flush;
  //std::cout << "\nfullSizeFn2 = " << fullSizeFn2 << std::flush;

  Magick::Image im1; im1.read( fullSizeFn1 );
  Magick::Image im2; im2.read( fullSizeFn2 );
  Magick::Image im2t;

  if (fullSizeExist) {
    // modify Hinit to account for scale change
    Magick::Image imSmall1; imSmall1.read( image_fn1 );
    Magick::Image imSmall2; imSmall2.read( image_fn2 );
    double sc1w= static_cast<double>(im1.columns())/imSmall1.columns();
    double sc2w= static_cast<double>(im2.columns())/imSmall2.columns();
    double sc1h= static_cast<double>(im1.rows())/imSmall1.rows();
    double sc2h= static_cast<double>(im2.rows())/imSmall2.rows();
    double sc21w= sc2w/sc1w, sc21h= sc2h/sc1h;
    Hinit.H[0]*= sc21w;
    Hinit.H[1]*= sc21w;
    Hinit.H[2]*= sc2w;
    Hinit.H[3]*= sc21h;
    Hinit.H[4]*= sc21h;
    Hinit.H[5]*= sc2h;
    xl*= sc1w; xu*= sc1w;
    yl*= sc1h; yu*= sc1h;
  }

  homography H= Hinit;

  uint32_t numFeats1, numFeats2; //, bestNInliers;
  float **descs1, **descs2;
  std::vector<ellipse> regions1, regions2;

  boost::thread *thread1, *thread2;

  // temp image file where transformed image is written in each iteration
  boost::filesystem::path tmp_dir = boost::filesystem::temp_directory_path() / "imcomp";
  tmp_dir = tmp_dir / "tmp";
  boost::filesystem::path fullSizeFn2_t = tmp_dir / boost::filesystem::unique_path("rr_register_%%%%-%%%%-%%%%-%%%%.jpg");
  std::cout << "\nfullSizeFn2_t = " << fullSizeFn2_t.string() << std::flush;

  // compute RootSIFT: image 1
  thread1= new boost::thread( featWorker( featGetterObj, fullSizeFn1, xl, xu, yl, yu, numFeats1, regions1, descs1 ) );

  bool firstGo= true, extractFinished1= false;
  uint32_t loopNum_= 0;

  matchesType inlierInds;

  // debug
  thread1->join();

  while (1){

    if (!firstGo){

      // compute RootSIFT: image 2
      std::string fn2 = fullSizeFn2_t.string();
      thread2= new boost::thread( featWorker( featGetterObj, fn2.c_str(), xl, xu, yl, yu, numFeats2, regions2, descs2 ) );

      // wait for feature extraction of image 1 to finish
      if (!extractFinished1){
        thread1->join();
        delete thread1;
        extractFinished1= true;
      }

      // wait for feature extraction of image 2 to finish
      thread2->join();
      delete thread2;

      // run RANSAC

      homography Hnew;

      detRansac::matchDesc(
                           bestNInliers,
                           descs1, regions1,
                           descs2, regions2,
                           featGetterObj->numDims(),
                           loopNum_>1?1.0:5.0, 0.0, 1000.0, static_cast<uint32_t>(4),
                           true, 0.81f, 100.0f,
                           &Hnew, &inlierInds
                           );
      bool success= bestNInliers>9;

      if (!success) {
        // indicate failure to match
        return;
      }

      // apply new H to current H (i.e. H= H * Hnew)
      {
        double Happlied[9];
        for (int i= 0; i<3; ++i)
          for (int j=0; j<3; ++j)
            Happlied[i*3+j]= H.H[i*3  ] * Hnew.H[  j] +
              H.H[i*3+1] * Hnew.H[3+j] +
              H.H[i*3+2] * Hnew.H[6+j];
        H.set(Happlied);
      }

    }

    H.normLast();

    // im2 -> im1 transformation
    double Hinv[9];
    H.getInverse(Hinv);
    homography::normLast(Hinv);

    // warp image 2 into image 1
    im2t= im2;

    // AffineProjection(sx, rx, ry, sy, tx, ty) <=> H=[sx, ry, tx; sy, rx, ty; 0 0 1]
    double MagickAffine[6]={Hinv[0],Hinv[3],Hinv[1],Hinv[4],Hinv[2],Hinv[5]};
    im2t.virtualPixelMethod(Magick::BlackVirtualPixelMethod);
    im2t.distort(Magick::AffineProjectionDistortion, 6, MagickAffine, false);

    firstGo= false;

    ++loopNum_;
    if (loopNum_>1) {
      break;
    }
    im2t.write( fullSizeFn2_t.string().c_str() );
  }

  delete []descs1;
  delete []descs2;
  boost::filesystem::remove( fullSizeFn2_t );

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

  //std::cout << "\nim1 = " << im1.columns() << "," << im1.rows() << std::flush;
  //std::cout << "\nim2 = " << im2.columns() << "," << im2.rows() << std::flush;

  im2t= im2;
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
    }
  }
  diff.write(diff_image);

  delete featGetterObj;
}

void registerImages::compute_homography(const char image1_fn[], const char image2_fn[],
                                        double x, double y, double w, double h,
                                        homography &h0 ) {
    featGetter *featGetterObj= new featGetter_standard( "hesaff-rootsift" );

    Magick::Image im1; im1.read( image1_fn );
    Magick::Image im2; im2.read( image2_fn );
    Magick::Image im2t;

    homography H = h0;
    uint32_t numFeats1, numFeats2, bestNInliers;
    float **descs1, **descs2;
    std::vector<ellipse> regions1, regions2;

    boost::thread *thread1, *thread2;

    // compute RootSIFT: image 1
    thread1 = new boost::thread( featWorker( featGetterObj, image1_fn, x, x+w, y, y+h, numFeats1, regions1, descs1 ) );


    bool firstGo= true, extractFinished1= false;
    uint32_t loopNum_= 0;

    boost::filesystem::path tmp_dir = boost::filesystem::temp_directory_path() / "imcomp";
    tmp_dir = tmp_dir / "tmp";
    boost::filesystem::path image2_fn_t = tmp_dir / boost::filesystem::unique_path("rr_register_%%%%-%%%%-%%%%-%%%%.jpg");
    matchesType inlierInds;

    while (1){
        if (!firstGo){
          // compute RootSIFT: image 2
          thread2 = new boost::thread( featWorker( featGetterObj, image2_fn_t.string().c_str(), x, x+w, y, y+h, numFeats2, regions2, descs2 ) );

            // wait for feature extraction of image 1 to finish
            if (!extractFinished1){
                thread1->join();
                delete thread1;
                extractFinished1= true;
            }

            // wait for feature extraction of image 2 to finish
            thread2->join();
            delete thread2;

            // run RANSAC

            homography Hnew;

            detRansac::matchDesc(
                //sameRandomObj,
                bestNInliers,
                descs1, regions1,
                descs2, regions2,
                featGetterObj->numDims(),
                loopNum_>1?1.0:5.0, 0.0, 1000.0, static_cast<uint32_t>(4),
                true, 0.81f, 100.0f,
                &Hnew, &inlierInds
                );

            bool success= bestNInliers>9;

            if (!success)
                break;

            // apply new H to current H (i.e. H= H * Hnew)
            {
                double Happlied[9];
                for (int i= 0; i<3; ++i)
                    for (int j=0; j<3; ++j)
                        Happlied[i*3+j]= H.H[i*3  ] * Hnew.H[  j] +
                                         H.H[i*3+1] * Hnew.H[3+j] +
                                         H.H[i*3+2] * Hnew.H[6+j];
                H.set(Happlied);
            }

        }

        H.normLast();

        // im2 -> im1 transformation
        double Hinv[9];
        H.getInverse(Hinv);
        homography::normLast(Hinv);

        // warp image 2 into image 1
        im2t= im2;
        // AffineProjection(sx, rx, ry, sy, tx, ty) <=> H=[sx, ry, tx; sy, rx, ty; 0 0 1]
        double MagickAffine[6]={Hinv[0],Hinv[3],Hinv[1],Hinv[4],Hinv[2],Hinv[5]};
        im2t.virtualPixelMethod(Magick::BlackVirtualPixelMethod);
        im2t.distort(Magick::AffineProjectionDistortion, 6, MagickAffine, false);

        firstGo= false;

        ++loopNum_;
        if ( loopNum_ > 1 ) {
/*
          double h[9];
          H.exportToDoubleArray(h);

          std::ostringstream s;
          s.precision(4);
          s << h[0] << " " << h[1] << " " << h[2] << " ";
          s << h[3] << " " << h[4] << " " << h[5] << " ";
          s << h[6] << " " << h[7] << " " << h[8];
          std::cout << s.str();
*/
          break;
        }

        im2t.write( image2_fn_t.string().c_str() );

    }

    descGetter::del(numFeats1, descs1);
    descGetter::del(numFeats2, descs2);
    boost::filesystem::remove( image2_fn_t );

    delete featGetterObj;
}

void
registerImages::findBBox2( double xl, double xu, double yl, double yu, homography const &H, double &xl2, double &xu2, double &yl2, double &yu2, uint32_t w2, uint32_t h2 ){

    ASSERT( fabs(H.H[8]-1.0)<1e-5 );

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
