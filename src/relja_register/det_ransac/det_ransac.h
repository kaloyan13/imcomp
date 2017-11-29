#ifndef _DET_RANSAC_H_
#define _DET_RANSAC_H_

// #define RANSAC_BINNING

#include <stdint.h>
#include <vector>
#include <algorithm>
#include <iostream>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>

#include "ellipse.h"
#include "homography.h"


typedef std::vector< std::pair<uint32_t, uint32_t> > matchesType;

class detRansac {
    
    private:
        class inlierFinder;
    
    public:
                
        static double
            matchDesc(
                
                uint32_t &nInliers,
                
                float const* const* desc1,
                std::vector<ellipse> const &ellipses1,
                float const* const* desc2,
                std::vector<ellipse> const &ellipses2,
                uint32_t nDims,
                
                double errorThr,
                double lowAreaChange, double highAreaChange,
                uint32_t nReest= 4,
                
                bool useLowe= true,
                float deltaSq= 0.81f,
                float epsilon= 100.0f,
                
                homography *H= NULL,
                matchesType *inlierInds= NULL
                
                );
        
        static double
            match(
                
                uint32_t &bestNInliers,
                
                std::vector<ellipse> const &ellipses1,
                std::vector<ellipse> const &ellipses2,
                matchesType const &putativeMatches,
                std::vector<double> const *PMweights,
                
                double errorThr,
                double lowAreaChange, double highAreaChange,
                uint32_t nReest= 4,
                
                homography *H= NULL,
                matchesType *inlierInds= NULL
                
                );
            
    private:
        
        static double
            bestScore( 
                inlierFinder **inlierFinders,
                std::vector<uint32_t> &Hbin,
                std::vector<homography> const &Hs,
                uint32_t &bestNInliers, homography &bestH,
                uint32_t bestNInliers_sofar, // don't pass by reference!
                uint32_t &globalNIter, uint32_t globalNPutativeMatches );
        
        inline static uint32_t
            getNStopping(double pFail, uint32_t nPutativeMatches, uint32_t bestNInliers){
                if (bestNInliers==nPutativeMatches) return 0;
                return std::min(
                    100000.0,
                    std::ceil(
                        std::log(pFail) /
                        std::log( 1.0 - static_cast<double>( std::max(static_cast<uint32_t>(4),bestNInliers) ) / nPutativeMatches )
                        )
                    );
            }
        
        static void
            getH( std::vector<ellipse> const &ellipses1,
                  std::vector<ellipse> const &ellipses2,
                  matchesType const &inliers, homography &H );
        
        static void
            normPoints( double *x, double *y, uint32_t n, homography &Hnorm );
        
        class sortH_helper{
            public:
                sortH_helper( std::vector<homography> *aHs, bool a ): Hs(aHs){
                    ind= (a?0:2);
                }
                std::vector<homography> *Hs;
                int ind;
                bool operator()(uint32_t i, uint32_t j){
                    return Hs->at(i).H[ind] < Hs->at(j).H[ind];
                }
        };
        
        
        
        class inlierFinder {
            
            public:
                
                inlierFinder(
                    std::vector<ellipse> const &aEllipses1,
                    std::vector<ellipse> const &aEllipses2,
                    matchesType const &aPutativeMatches,
                    std::vector<double> const &aPMweights,
                    double aErrorThr,
                    double aLowAreaChange, double aHighAreaChange);
                
                ~inlierFinder();
                
                double
                    getScore( homography const &H, uint32_t &nInliers, matchesType *inliers= NULL );
                
                inline double
                    getMaxScore(){ return static_cast<double>(nPutativeMatches); }
                
                inline uint32_t
                    getMaxInliers(){ return nPutativeMatches; }
                
            private:
                
                uint32_t nIter;
                
                matchesType const *putativeMatches;
                std::vector<double> const *PMweights;
                double errorThrSq, lowAreaChangeSq, highAreaChangeSq;
                
                double *x1, *y1, *x2, *y2, *areaDiffSq;
                
                std::vector<uint32_t> point1Used, point2Used;
                
                uint32_t nPutativeMatches;
                            
        };
        
        
        
        class threadSafeRandShuffle {
            
            public:
                
                threadSafeRandShuffle( uint32_t seed ) : gen(seed) { }
                
                template <class T>
                void
                    shuffle( typename std::vector<T>::iterator itBegin,
                             typename std::vector<T>::iterator itEnd ){
                    uint32_t n= itEnd-itBegin;
                    uint32_t i= n-1;
                    for (--itEnd; itBegin!=itEnd; --itEnd )
                        std::swap( *itEnd, *(itBegin+getRand(i+1)) );
                }
                
                template <class T>
                inline static void
                    shuffle( typename std::vector<T>::iterator itBegin,
                             typename std::vector<T>::iterator itEnd,
                             uint32_t seed ){
                        threadSafeRandShuffle shuffle_obj(seed);
                        shuffle_obj.shuffle<T>( itBegin, itEnd );
                    }
                                
            private:
                
                inline uint32_t
                    getRand( uint32_t n ){
                        boost::uniform_int<> dist(0, n-1);
                        boost::variate_generator<boost::mt19937&, boost::uniform_int<> > number(gen, dist);
                        return number();
                    }
                
                boost::mt19937 gen;
                
        };
        
        
        
        class binSorter {
            
            public:
                
                binSorter( std::vector<matchesType> &aBinPMs ): binPMs(&aBinPMs){}
                bool operator()( uint32_t i, uint32_t j ){ return binPMs->at(i).size() > binPMs->at(j).size(); }
                std::vector<matchesType> *binPMs;
            
        };
        
        
};

#endif
