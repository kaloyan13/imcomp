#include "det_ransac.h"

#include <Eigen/Dense>

#include "putative.h"



inline double mysqr( double x ){ return x*x; }



double
detRansac::matchDesc(
    
    uint32_t &nInliers,
    
    float const*const* desc1,
    std::vector<ellipse> const &ellipses1,
    float const*const* desc2,
    std::vector<ellipse> const &ellipses2,
    uint32_t nDims,
    
    double errorThr,
    double lowAreaChange, double highAreaChange,
    uint32_t nReest,
    
    bool useLowe,
    float deltaSq,
    float epsilon,
    
    homography *H,
    matchesType *inlierInds
    
    ){
    
    std::vector< std::pair<uint32_t, uint32_t> > putativeMatches;
    
    putative_desc<float>::getPutativeMatches( desc1, ellipses1.size(), desc2, ellipses2.size(), nDims, putativeMatches, useLowe, deltaSq, epsilon );
    
    return detRansac::match( nInliers, ellipses1, ellipses2, putativeMatches, NULL, errorThr, lowAreaChange, highAreaChange, nReest, H, inlierInds);
    
}



double
detRansac::match(
    
    uint32_t &bestNInliers,
    
    std::vector<ellipse> const &ellipses1,
    std::vector<ellipse> const &ellipses2,
    matchesType const &putativeMatches,
    std::vector<double> const *PMweights,
    
    double errorThr,
    double lowAreaChange, double highAreaChange,
    uint32_t nReest,
    
    homography *H,
    matchesType *inlierInds
    
    ){
    
    bestNInliers= 0;
    
    uint32_t nPutativeMatches= putativeMatches.size();
    
    if (nPutativeMatches<3)
        return 0;
    
    //------- prepare
    
    bool delWeights= false;
    if (PMweights==NULL) {
        PMweights= new std::vector<double>( nPutativeMatches, 1.0 );
        delWeights= true;
    }
    
    //------- generate Hs
    
    std::vector<homography> Hs;
    Hs.reserve( nPutativeMatches );
    
    homography Hident; Hident.setIdentity();
    homography bestH= Hident;
    
    detRansac::inlierFinder inlierFinder_obj_All( ellipses1, ellipses2, putativeMatches, *PMweights, errorThr, lowAreaChange, highAreaChange );
    
    double score= 0.0, currScore;
    bestNInliers= 0;
    score= inlierFinder_obj_All.getScore( bestH, bestNInliers, NULL );
    
    for (std::vector< std::pair<uint32_t, uint32_t> >::const_iterator itPM= putativeMatches.begin();
         itPM!=putativeMatches.end();
         ++itPM){
        
        Hs.push_back( homography( ellipses1[itPM->first], ellipses2[itPM->second] ) );
        
    }
    
    #ifdef RANSAC_BINNING
    uint32_t numDivs= std::min( 5.0, std::max( 1.0, std::ceil( std::sqrt(std::sqrt( static_cast<double>(Hs.size()) )) ) ) );
    if (putativeMatches.size()<100)
        numDivs= 1;
    #endif
    
    uint32_t globalNIter= 0, currNInliers= 0;
    homography currBestH;
    
    #if 0
    // not thread safe as the random number generator has an internal state
    std::srand(0);
    std::random_shuffle(Hs.begin()+1, Hs.end());
    #else
    threadSafeRandShuffle::shuffle<homography>( Hs.begin(), Hs.end(), 0 );
    #endif
    
    inlierFinder **inlierFinders= NULL;
    uint32_t numBins;
    std::vector<uint32_t> Hbin( Hs.size() );
    std::vector<matchesType> *binnedPutativeMatches= NULL;
    bool doingBinning;
    
    #ifdef RANSAC_BINNING
    if (numDivs<=1){
    #endif
        
        // normal ransac, no binning
        doingBinning= false;
        numBins= 1;
        inlierFinders= new inlierFinder*[1];
        inlierFinders[0]= &inlierFinder_obj_All;
        std::fill(Hbin.begin(), Hbin.end(), 0);
            
    #ifdef RANSAC_BINNING
    } else {
        
        //------- compute bins based on a, tx
        // critical for all this is that Haff[0][1]=0 (i.e. gravity) !!
        
        doingBinning= true;
        
        numBins= numDivs*numDivs;
        uint32_t numPerBin= std::ceil( static_cast<double>(Hs.size())/numBins ); // ceil important as otherwise leftovers!
        uint32_t nCurrBin;
        
        // each bin: pair( pair(a_min, a_max), pair(tx_min,tx_max) )
        typedef std::pair< std::pair<double,double>, std::pair<double,double> > binType;
        std::vector<binType> bins;
        bins.reserve( numBins );
        std::vector<uint32_t> Hind;
        Hind.reserve( Hs.size() );
        for (uint32_t i_=0; i_<Hs.size(); ++i_) Hind.push_back(i_);
        
        //TODO more ballanced binning
        std::sort( Hind.begin(), Hind.end(), sortH_helper(&Hs,true) );
        
        sortH_helper sortH_helper_tx(&Hs,false);
        double *currH= NULL;
        uint32_t iBin= 0;
        
        uint32_t numH= Hind.size();
        for (uint32_t iiH= 0; iiH < numH; ){
            
            std::sort( Hind.begin()+iiH,
                       Hind.begin()+iiH+ std::min( numH-iiH, numPerBin*numDivs ),
                       sortH_helper_tx );
            
            for (uint32_t nBinInBin= 0; nBinInBin<numDivs && iiH < numH; ++nBinInBin, ++iBin){
                
                Hbin[ Hind[iiH] ]= iBin;
                currH= Hs[ Hind[iiH] ].H;
                assert( currH[1] < 1e-4 ); // requirement for this method!
                binType currBin= std::make_pair(
                    std::make_pair(currH[0],currH[0]),
                    std::make_pair(currH[2],currH[2]) );
                
                nCurrBin= 1;
                for (++iiH; iiH < numH && nCurrBin<numPerBin; ++iiH, ++nCurrBin){
                    Hbin[ Hind[iiH] ]= iBin;
                    currH= Hs[ Hind[iiH] ].H;
                    assert( currH[1] < 1e-4 ); // requirement for this method!
                    currBin.first.first  = std::min(currBin.first.first,   currH[0]);
                    currBin.first.second = std::max(currBin.first.second,  currH[0]);
                    currBin.second.first = std::min(currBin.second.first,  currH[2]);
                    currBin.second.second= std::max(currBin.second.second, currH[2]);
                }
                
                bins.push_back( currBin );
                
            }
            
        }
        currH= NULL;
        
        numBins= bins.size();
        
        //------- bin putative matches based on a, tx
        
        double errorThrEff, x1, x2, al, au, txl, txu;
        iBin= 0;
        
        binnedPutativeMatches= new std::vector<matchesType>( numBins );
        
        for (matchesType::const_iterator itPM= putativeMatches.begin();
             itPM!=putativeMatches.end();
             ++itPM){
            
            x1= ellipses1[ itPM->first  ].x;
            x2= ellipses2[ itPM->second ].x;
            assert(x1>0);
            iBin= 0;
            for (std::vector<binType>::iterator itB= bins.begin();
                 itB!=bins.end();
                 ++itB, ++iBin){
                
                al = itB->first.first;  au = itB->first.second;
                txl= itB->second.first; txu= itB->second.second;
                errorThrEff= errorThr / sqrt(1 + 1.0/(au*au));
                
                if ( -errorThrEff < au*x1+txu-x2 && al*x1+txl-x2 < errorThrEff )
                    binnedPutativeMatches->at( iBin ).push_back( *itPM );
                
            }
        }
        
        //------- init inlier finders
        
        //!!TODO incorporate PMweights here
        std::cout<<"\n\tIMPLEMENT PMweights!\n";
        inlierFinders= new inlierFinder*[numBins];
        
        for (iBin= 0; iBin<numBins; ++iBin)
            inlierFinders[iBin]= new inlierFinder( ellipses1, ellipses2, binnedPutativeMatches->at(iBin), errorThr, lowAreaChange, highAreaChange );
        
    }
    #endif //ifdef RANSAC_BINNING
    
    
    //------- find best inliers
    
    currScore= detRansac::bestScore( inlierFinders, Hbin, Hs, currNInliers, currBestH, bestNInliers, globalNIter, putativeMatches.size() );
    
    if (currNInliers>3 && currScore>score){
        score= currScore;
        bestNInliers= currNInliers;
        bestH= currBestH;
    }
    
    
    //------- cleanup inlier finders
    
    assert(doingBinning || numBins==1);
    if (doingBinning){
        for (uint32_t iBin= 0; iBin<numBins; ++iBin)
            delete inlierFinders[iBin];
        delete binnedPutativeMatches;
    }
    delete []inlierFinders;
    
    
    if (bestNInliers > 3){
        
        matchesType bestInliers;
        bestInliers.reserve(bestNInliers);
        uint32_t nInliers_new;
        inlierFinder_obj_All.getScore( bestH, nInliers_new, &bestInliers );
        
        //------- reestimate
        
        homography H_new= bestH;
        double score_new;
        matchesType inliers_new;
        inliers_new.reserve( bestNInliers );
                
        inliers_new= bestInliers;
        
        uint32_t iReest;
        for (iReest= 0; nInliers_new>3 && iReest<nReest; ++iReest){
            detRansac::getH( ellipses1, ellipses2, inliers_new, H_new );
            score_new= inlierFinder_obj_All.getScore( H_new, nInliers_new, &inliers_new );
            if (nInliers_new>3 && score_new>score){
                score= score_new;
                bestNInliers= nInliers_new;
                bestH= H_new;
                bestInliers= inliers_new; // don't swap as next getH in next needs it!
            }
        }
        
        if (inlierInds)
            inlierInds->swap( bestInliers );
        
        if (H)
            *H= bestH;
        
    }
    
    //------- cleanup
    
    if (delWeights){
        delete PMweights;
        PMweights= NULL;
    }
    
    return score;
    
}



void
detRansac::getH( std::vector<ellipse> const &ellipses1,
                 std::vector<ellipse> const &ellipses2,
                 matchesType const &inliers, homography &H ){
    
    Eigen::Matrix<double, Eigen::Dynamic, 7> A( inliers.size()*2, 7 );
    
    double *x1, *y1, *x2, *y2, *_temp;
    ellipse::getCentres( ellipses1, ellipses2, inliers, x1, y1, x2, y2, _temp );
    uint32_t nInliers= inliers.size();
    
    // normalize points
    homography Hnorm1, Hnorm2;
    normPoints(x1, y1, nInliers, Hnorm1);
    normPoints(x2, y2, nInliers, Hnorm2);
    double Hnorm2inv[9];
    Hnorm2.getInverse(Hnorm2inv);
    
    // fit homography
    uint32_t i=0;
    
    for (matchesType::const_iterator itIn= inliers.begin();
         itIn!=inliers.end();
         ++itIn, ++i){
        
        A.coeffRef( i*2   , 0 )=    0.0;
        A.coeffRef( i*2   , 1 )=    0.0;
        A.coeffRef( i*2   , 2 )=    0.0;
        A.coeffRef( i*2   , 3 )=  x1[i];
        A.coeffRef( i*2   , 4 )=  y1[i];
        A.coeffRef( i*2   , 5 )=    1.0;
        A.coeffRef( i*2   , 6 )= -y2[i];
        
        A.coeffRef( i*2+1 , 0 )= -x1[i];
        A.coeffRef( i*2+1 , 1 )= -y1[i];
        A.coeffRef( i*2+1 , 2 )=   -1.0;
        A.coeffRef( i*2+1 , 3 )=    0.0;
        A.coeffRef( i*2+1 , 4 )=    0.0;
        A.coeffRef( i*2+1 , 5 )=    0.0;
        A.coeffRef( i*2+1 , 6 )=  x2[i];
        
    }
    
    delete []x1; delete []y1; delete []x2; delete []y2; delete []_temp;
    
    typedef Eigen::Matrix<double, 7, 7> matrix7x7;
    matrix7x7 AtA;
    AtA.noalias()= A.transpose() * A;
    Eigen::SelfAdjointEigenSolver<matrix7x7> sol( AtA );
    Eigen::Matrix<double,7,1> x= sol.eigenvectors().col(0);
    
    H.H[0]= x.coeff(0,0); H.H[1]= x.coeff(1,0); H.H[2]= x.coeff(2,0);
    H.H[3]= x.coeff(3,0); H.H[4]= x.coeff(4,0); H.H[5]= x.coeff(5,0);
    H.H[6]=          0.0; H.H[7]=          0.0; H.H[8]= x.coeff(6,0);
    H.normLast();
    
    // denormalize H: H= Hnorm2inv * Hnorm * Hnorm1
    
    Eigen::Matrix<double, 3, 3> Hnorm2inv_, Hnorm1_, Hnorm_;
    for (int i=0; i<3; ++i)
        for (int j=0; j<3; ++j){
            Hnorm_.coeffRef(i,j)= H.H[i*3+j];
            Hnorm1_.coeffRef(i,j)= Hnorm1.H[i*3+j];
            Hnorm2inv_.coeffRef(i,j)= Hnorm2inv[i*3+j];
        }
    Eigen::Matrix<double, 3, 3> H_;
    H_.noalias()= Hnorm2inv_ * Hnorm_ * Hnorm1_;
    
    for (int i=0; i<3; ++i)
        for (int j=0; j<3; ++j)
            H.H[i*3+j]= H_.coeff(i,j);
    
}



void
detRansac::normPoints( double *x, double *y, uint32_t n, homography &Hnorm ){
    
    double EX= 0, EY= 0, stdX, stdY, EX2= 0, EY2= 0;
    uint32_t i;
    
    // get mean/variance
    for (i= 0; i<n; ++i){
        EX+= x[i];
        EY+= y[i];
        EX2+= mysqr(x[i]);
        EY2+= mysqr(y[i]);
    }
    EX/=n; EY/=n;
    EX2/=n; EY2/=n;
    // max is there just in case std=0 (as later norm=NaN in this case), in this case the dimension gets mapped to 0 (as mean is 0) so scaling makes no difference.. This is not nice however, as it means the problem is illposed
    stdX= std::max( sqrt(EX2-mysqr(EX)), 1e-4 );
    stdY= std::max( sqrt(EY2-mysqr(EY)), 1e-4 );
    
    double normX= sqrt(2.0)/stdX, normY= sqrt(2.0)/stdY;
    
    Hnorm.setIdentity();
    Hnorm.H[0]= normX; Hnorm.H[2]= -EX*normX;
    Hnorm.H[4]= normY; Hnorm.H[5]= -EY*normY;
    
    // normalize points
    for (i= 0; i<n; ++i){
        x[i]= (x[i]-EX)*normX;
        y[i]= (y[i]-EY)*normY;
    }
       
    
}



double
detRansac::bestScore(
    inlierFinder **inlierFinders,
    std::vector<uint32_t> &Hbin,
    std::vector<homography> const &Hs,
    uint32_t &bestNInliers, homography &bestH,
    uint32_t bestNInliers_sofar,
    uint32_t &globalNIter, uint32_t globalNPutativeMatches ){
    
    
    //------- find inliers
    
    static const double pFail= 0.001;
    double bestScore= 0;
    bestNInliers= 0;
    
    double score;
    uint32_t nInliers, iH= 0;
    
    for (std::vector<homography>::const_iterator itH= Hs.begin();
         itH!=Hs.end() && globalNIter < detRansac::getNStopping(pFail, globalNPutativeMatches, bestNInliers_sofar);
         ++itH, ++globalNIter, ++iH){
        
        #ifdef RANSAC_BINNING
        if (bestNInliers_sofar >= inlierFinders[Hbin[iH]]->getMaxInliers() )
            continue;
        #endif
        score= inlierFinders[Hbin[iH]]->getScore( *itH, nInliers, NULL );
        
        if (nInliers>3 && score>bestScore) {
            bestNInliers= nInliers;
            bestNInliers_sofar= bestNInliers;
            bestScore= score;
            bestH= *itH;
        }
        
    }
    
    return bestScore;
    
}



detRansac::inlierFinder::inlierFinder(
    std::vector<ellipse> const &aEllipses1,
    std::vector<ellipse> const &aEllipses2,
    matchesType const &aPutativeMatches,
    std::vector<double> const &aPMweights,
    double aErrorThr,
    double aLowAreaChange, double aHighAreaChange) :
    
    nIter(0),
    putativeMatches(&aPutativeMatches), PMweights(&aPMweights),
    errorThrSq(mysqr(aErrorThr)),
    lowAreaChangeSq(mysqr(aLowAreaChange)),
    highAreaChangeSq(mysqr(aHighAreaChange)),
    nPutativeMatches(aPutativeMatches.size()) {
    
    //------- get largest pIDs
    
    uint32_t maxpID1= 0, maxpID2= 0;
    
    for (matchesType::const_iterator itPM=putativeMatches->begin();
         itPM != putativeMatches->end();
         ++itPM){
        maxpID1= std::max(maxpID1, itPM->first);
        maxpID2= std::max(maxpID2, itPM->second);
    }
    point1Used.clear(); point1Used.resize(maxpID1+1,0);
    point1Used.clear(); point2Used.resize(maxpID2+1,0);
    
    ellipse::getCentres( aEllipses1, aEllipses2, *putativeMatches, x1, y1, x2, y2, areaDiffSq );
    
}



detRansac::inlierFinder::~inlierFinder(){
    delete []x1; delete []y1; delete []x2; delete []y2; delete []areaDiffSq;
}



double
detRansac::inlierFinder::getScore( homography const &H, uint32_t &nInliers, matchesType *inliers ){
    
    
    double score, detASq;
        
    score= 0.0;
    nInliers= 0;
    if (inliers)
        inliers->clear();
    
    detASq= mysqr( H.getDetAffine() );
    if (detASq<1e-4) return 0.0;
    
    double error;
    uint32_t pID1, pID2, iPM;
    double x, y, xi, yi;
    double lowAreaChangeSqByD =  lowAreaChangeSq / detASq;
    double highAreaChangeSqByD= highAreaChangeSq / detASq;
    
    double Hinv[9];
    H.getInverse( Hinv );
    
    ++nIter;
    
    iPM= 0;
    for (matchesType::const_iterator itPM= putativeMatches->begin();
         itPM!=putativeMatches->end();
         ++itPM, ++iPM){
        
        pID1= itPM->first;
        pID2= itPM->second;
        if (point1Used[ pID1 ]==nIter || point2Used[ pID2 ]==nIter)
            continue;
        
        homography::affTransform( H.H , x1[iPM], y1[iPM], x , y );
        homography::affTransform( Hinv, x2[iPM], y2[iPM], xi, yi );
        
        error= mysqr( x1[iPM]-xi ) + mysqr( y1[iPM]-yi ) +
               mysqr( x2[iPM]-x  ) + mysqr( y2[iPM]-y  );
        
        if (error < errorThrSq){
            /*
            areaChangeSq= detASq * ellipses2[ pID2 ].getPropAreaSq() / ellipses1[ pID1 ].getPropAreaSq();
            if ( areaChangeSq > lowAreaChangeSq && areaChangeSq < highAreaChangeSq ){
            */
            if (areaDiffSq[iPM] > lowAreaChangeSqByD && areaDiffSq[iPM] < highAreaChangeSqByD) {
                score+= PMweights->at(iPM);
                ++nInliers;
                point1Used[ pID1 ]= nIter;
                point2Used[ pID2 ]= nIter;
                if (inliers)
                    inliers->push_back(std::make_pair(pID1,pID2));
            }
        }
        
    }
    
    return score;
    
}
