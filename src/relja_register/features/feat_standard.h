#ifndef _FEAT_STANDARD_H_
#define _FEAT_STANDARD_H_

#include <stdexcept>

#include "feat_getter.h"
#include "desc_to_hell.h"
#include "ellipse.h"



// Hessian-Affine detector by Krystian Mikolajczyk
class reg_KM_HessAff : public regionGetter {
public: void getRegs( const char filename_jpeg[], uint32_t &numRegs, std::vector<ellipse> &regions );
};



// SIFT by Krystian Mikolajczyk
class desc_KM_SIFT : public descGetter {
    public:
        void getDescs(const char filename_jpeg[], std::vector<ellipse> &regions, uint32_t &numFeats, float **&descs );
        uint32_t numDims() const { return 128; }
};


class featGetter_standard : public featGetter {
    
    public:
        
        featGetter_standard( const char id[] ) :
            featGetterObj(NULL), regionGetterObj(NULL), descGetterObj(NULL) {
            
            if ( strcmp(id,"hesaff-sift") == 0 ){
                regionGetterObj= new reg_KM_HessAff();
                descGetterObj= new desc_KM_SIFT();
                featGetterObj= new splitRegDesc( regionGetterObj, descGetterObj );
            } else if ( strcmp(id,"hesaff-rootsift") == 0 ) {
                regionGetterObj= new reg_KM_HessAff();
                descGetter *dg= new desc_KM_SIFT();
                descGetterObj= new descToHell(dg,true);
                featGetterObj= new splitRegDesc( regionGetterObj, descGetterObj );
            } else {
                throw std::runtime_error( "Unknown standard featGetter" );
            }
            
        }
        
        ~featGetter_standard(){
            
            delete featGetterObj;
            if (regionGetterObj!=NULL)
                delete regionGetterObj;
            if (descGetterObj!=NULL)
                delete descGetterObj;
            
        }
        
        void
            getFeats( const char fileName[], uint32_t &numFeats, std::vector<ellipse> &regions, float **&descs ){
            featGetterObj->getFeats( fileName, numFeats, regions, descs );
        }
        
        uint32_t
            numDims() const { return featGetterObj->numDims(); }
    
    private:
        
        featGetter *featGetterObj;
        regionGetter *regionGetterObj;
        descGetter *descGetterObj;
        
    
};

#endif
