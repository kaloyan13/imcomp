//
// Author: Abhishek Dutta <adutta@robots.ox.ac.uk>
// Oct. 10, 2017
//
// portions of code borrowed from register_images.cpp

#include "register_images.h"

#include <algorithm>
#include <math.h>
#include <vector>
#include <fstream>

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

void get_ransac_inliers(const char image1_fn[], const char image2_fn[], const char inliers_out_fn[]) {
  featGetter *featGetterObj= new featGetter_standard( "hesaff-rootsift" );

  Magick::Image im1; im1.read( image1_fn );
  Magick::Image im2; im2.read( image2_fn );
  Magick::Image im2t;

  homography H;
  uint32_t bestNInliers;

  uint32_t numFeats1, numFeats2; //, bestNInliers;
  float **descs1, **descs2;
  std::vector<ellipse> regions1, regions2;

  boost::thread *thread1, *thread2;

  // match features of full image
  double xl = 0;
  double yl = 0;
  double xu = im1.columns();
  double yu = im1.rows();

  // compute RootSIFT: image 1
  thread1 = new boost::thread( featWorker( featGetterObj, image1_fn, xl, xu, yl, yu, numFeats1, regions1, descs1 ) );

  bool firstGo= true, extractFinished1= false;
  uint32_t loopNum_= 0;

  boost::filesystem::path fullSizeFn2_t= boost::filesystem::temp_directory_path() / boost::filesystem::unique_path("rr_register_%%%%-%%%%-%%%%-%%%%.jpg");

  matchesType inlierInds;

  while (1){

    if (!firstGo){

      // compute RootSIFT: image 2

      thread2= new boost::thread( featWorker( featGetterObj, image2_fn, xl, xu, yl, yu, numFeats2, regions2, descs2 ) );

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
        std::cout << "\nfailed to find sufficient inliers (inliers = " << bestNInliers << ")" << std::flush;
        return;
      }

      // apply new H to current H (i.e. H= H * Hnew)
      {
        double Happlied[9];
        for (int i= 0; i<3; ++i) {
          for (int j=0; j<3; ++j) {
            Happlied[i*3 + j]= H.H[i*3] * Hnew.H[j] + H.H[i*3+1] * Hnew.H[3+j] + H.H[i*3+2] * Hnew.H[6+j];
          }
        }
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

    im2t.write( fullSizeFn2_t.c_str() );
  }

  std::ofstream f(inliers_out_fn, std::ofstream::out);

  // print the coordinates of ellipse center for inliers
  //typedef std::vector< std::pair<uint32_t, uint32_t> > matchesType;
  std::cout << "\nbestNInliers = " << bestNInliers << std::flush;

  double Hinv[9];
  H.getInverse(Hinv);
  homography::normLast(Hinv);
  std::cout << "\nH = [" << Hinv[0] << "," << Hinv[1] << "," << Hinv[2] << "," << Hinv[3] << "," << Hinv[4] << "," << Hinv[5] << "]" << std::flush;
  std::cout << "\nWriting " << inlierInds.size() << " inliers matching region centers to " << inliers_out_fn << std::flush;
  f << "#" << image1_fn << "," << image2_fn;
  for ( unsigned int i=0; i<inlierInds.size(); i++ ) {
    uint32_t region1_index = inlierInds[i].first;
    uint32_t region2_index = inlierInds[i].second;

    ellipse r1 = regions1[region1_index];
    ellipse r2 = regions2[region2_index];
    //std::cout << "\n[" << i << "] (" << r1.x << "," << r1.y << "," << r1.a << "," << r1.b << "," << r1.c << ") -> (" << r2.x << "," << r2.y << "," << r2.a << "," << r2.b << "," << r2.c << ")" << std::flush;
    //std::cout << "\n[" << i << "] (" << r1.x << "," << r1.y << ") -> (" << r2.x << "," << r2.y << ")" << std::flush;
    f << "\n" << r1.x << "," << r1.y << "," << r2.x << "," << r2.y;
  }
  f.close();
  std::cout << "\nDone" << std::flush;

  // cleanup
  delete []descs1;
  delete []descs2;
  boost::filesystem::remove( fullSizeFn2_t );
  delete featGetterObj;
}


int main(int argc, char* argv[]) {
  if ( argc != 4 ) {
    std::cout << "\nUsage: " << argv[0] << " file1.jpg file2.jpg inliers_out.csv" << std::endl;
    return 0;
  }
  get_ransac_inliers(argv[1], argv[2], argv[3]);
  std::cout << std::endl;
}

