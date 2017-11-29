#ifndef _DESC_TO_HELL_H_
#define _DESC_TO_HELL_H_

#include <math.h>

#include "feat_getter.h"



class descToHell : public descGetter {
    
    public:
        
        descToHell( descGetter *aDescGetter, bool aDelChild ) : desc(aDescGetter), delChild(aDelChild)
            {}
        
        ~descToHell(){
            if (delChild)
                delete desc;
        }
        
        void
            getDescs( const char fileName[], std::vector<ellipse> &regions, uint32_t &numDescs, float **&descs ){
                desc->getDescs(fileName, regions, numDescs, descs);
                convertToHell( numDims(), numDescs, descs );
            }
        
        static void
            convertToHell( uint32_t numDims, uint32_t numDescs, float **descs ){
                
                uint32_t iDim;
                float l1norm;
                
                for (uint32_t iD= 0; iD<numDescs; ++iD){
                    l1norm= 0.0;
                    for (iDim= 0; iDim<numDims; ++iDim)
                        l1norm+= descs[iD][iDim];
                    l1norm= (l1norm>1e-6)?l1norm:1;
                    for (iDim= 0; iDim<numDims; ++iDim)
                        descs[iD][iDim]= sqrt( descs[iD][iDim] / l1norm );
                }
                
            }
        
        uint32_t
            numDims() const {
                return desc->numDims();
            }
    
    private:
        
        descGetter *desc;
        bool delChild;
    
};

#endif
