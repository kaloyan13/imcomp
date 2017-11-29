#ifndef _FEAT_GETTER_H_
#define _FEAT_GETTER_H_

#include <iostream>
#include <stdint.h>
#include <vector>

#include <ellipse.h>


class descGetter {
    
    public:
        
        virtual void
            getDescs( const char fileName[], std::vector<ellipse> &regions, uint32_t &numFeats, float **&descs ) =0;
        
        virtual uint32_t
            numDims() const =0;
        
        virtual ~descGetter()
            {}
        
        static void
            del( uint32_t numDescs, float **&descs ){
                if (numDescs<=0)
                    return;
                for (uint32_t iD=0; iD<numDescs; ++iD)
                    delete []descs[iD];
                delete []descs;
            }
    
};



class regionGetter {
    
    public:
        
        virtual void
            getRegs( const char fileName[], uint32_t &numRegs, std::vector<ellipse> &regions ) =0;
        
        virtual ~regionGetter()
            {}
    
};



class featGetter {
    
    public:
        
        virtual void
            getFeats( const char fileName[], uint32_t &numFeats, std::vector<ellipse> &regions, float **&descs ) =0;
        
        virtual ~featGetter()
            {}
        
        virtual uint32_t
            numDims() const =0;
        
        virtual void
            getFeats( const char fileName[], uint32_t xl, uint32_t xu, uint32_t yl, uint32_t yu, uint32_t &numFeats, std::vector<ellipse> &regions, float **&descs );
    
};


class splitRegDesc : public featGetter {
    
    public:
        
        splitRegDesc( regionGetter *aRegionGetterObj, descGetter *aDescGetterObj ) : regionGetterObj(aRegionGetterObj), descGetterObj(aDescGetterObj)
            {}
        
        void
            getFeats( const char fileName[], uint32_t &numFeats, std::vector<ellipse> &regions, float **&descs ){
                regionGetterObj->getRegs( fileName, numFeats, regions );
                descGetterObj->getDescs( fileName, regions, numFeats, descs );
            }
        
        uint32_t
            numDims() const { return descGetterObj->numDims(); }
    
    private:
        regionGetter *regionGetterObj;
        descGetter *descGetterObj;
    
};


#endif