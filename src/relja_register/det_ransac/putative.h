#ifndef _PUTATIVE_H_
#define _PUTATIVE_H_

#include <vector>

#include <stdint.h>
#include <limits>
#include <assert.h>

#include "jp_dist2.hpp"



template<class DescType>
class putative_desc {
    
    public:
        
        static void
            getPutativeMatches(
                    DescType const*const* desc1, uint32_t size1, DescType const*const* desc2, uint32_t size2, uint32_t nDims, std::vector< std::pair<uint32_t, uint32_t> > &putativeMatches, bool useLowe= true, float deltaSq= 0.81f, float epsilon= 100.0f );
                    
};



template<class DescType>
void
putative_desc<DescType>::getPutativeMatches(
    DescType const*const* desc1, uint32_t size1, DescType const*const* desc2, uint32_t size2, uint32_t nDims, std::vector< std::pair<uint32_t, uint32_t> > &putativeMatches, bool useLowe, float deltaSq, float epsilon ) {
    
    float dsq;
    
    if (!useLowe) { // Use the epsilon measure.
        
        for (uint32_t i=0; i<size1 ; ++i) {
            for (uint32_t j=0; j<size2 ; ++j) {
                dsq = jp_dist_l2(desc1[i], desc2[j], nDims);
                if (dsq < (epsilon*epsilon))
                    putativeMatches.push_back(std::make_pair(i, j));
            }
        }
      
    } else {
        
        static const int numnn = 2;
        std::pair<uint32_t,float> nns[numnn+1];
        int k;
        
        for (uint32_t i=0; i<size1; ++i) {
            
            nns[0] = nns[1] = nns[2] = std::make_pair(-1, std::numeric_limits<float>::max());
            for (uint32_t j=0; j<size2; ++j) {
                
                // One potential improvement is to copy desc1 into an aligned
                // buffer...
                dsq = jp_dist_l2(desc1[i], desc2[j], nDims);
                
                if (dsq < nns[numnn-1].second) {
                    for (k=numnn; k>0 && nns[k-1].second > dsq; --k)
                        nns[k] = nns[k-1];
                    nns[k] = std::make_pair(j, dsq);
                }
                
            }
            assert(nns[0].second <= nns[1].second);
            if ((nns[0].second/nns[1].second) < deltaSq) {
                putativeMatches.push_back(std::make_pair(i, nns[0].first));
            }
            
        }
        
    }
    
}




#endif
