//
// Author: Abhishek Dutta <adutta@robots.ox.ac.uk>
// Oct. 13, 2017
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

void get_putative_matches(const char image1_fn[], const char image2_fn[], const char putative_matches_fn[]) {
  featGetter *featGetterObj= new featGetter_standard( "hesaff-rootsift" );

  Magick::Image im1; im1.read( image1_fn );
  Magick::Image im2; im2.read( image2_fn );

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

  // compute RootSIFT: image 2
  thread2= new boost::thread( featWorker( featGetterObj, image2_fn, xl, xu, yl, yu, numFeats2, regions2, descs2 ) );

  // wait for feature extraction of image 1 and 2 to finish
  thread1->join(); delete thread1;
  thread2->join(); delete thread2;

  // get putative matches based on
  // 1. nearest neighbourhood
  // 2. Lowe's second nearest neighbour test
  std::vector< std::pair<uint32_t, uint32_t> > putative_matches;
  bool use_lowe = true;
  float delta2 = 0.81f;
  float epsilon = 100.0f;
  putative_desc<float>::getPutativeMatches( descs1, regions1.size(), descs2, regions2.size(), featGetterObj->numDims(), putative_matches, use_lowe, delta2, epsilon );

  std::ofstream f(putative_matches_fn, std::ofstream::out);

  // print the coordinates of ellipse center for inliers
  std::cout << "\nWriting " << putative_matches.size() << " putative matching region centers to " << putative_matches_fn << std::flush;
  f << "#" << image1_fn << "," << image2_fn;
  for ( unsigned int i=0; i<putative_matches.size(); i++ ) {
    uint32_t region1_index = putative_matches[i].first;
    uint32_t region2_index = putative_matches[i].second;

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
  delete featGetterObj;
}


int main(int argc, char* argv[]) {
  if ( argc != 4 ) {
    std::cout << "\nUsage: " << argv[0] << " file1.jpg file2.jpg putative_matches.csv" << std::endl;
    return 0;
  }
  get_putative_matches(argv[1], argv[2], argv[3]);
  std::cout << std::endl;
}

