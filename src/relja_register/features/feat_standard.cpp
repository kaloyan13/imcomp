/*
==== Author:

Relja Arandjelovic (relja@robots.ox.ac.uk)
Visual Geometry Group,
Department of Engineering Science
University of Oxford

==== Copyright:

The library belongs to Relja Arandjelovic and the University of Oxford.
No usage or redistribution is allowed without explicit permission.
*/

#include "feat_standard.h"

#include <string>
#include <fstream>

#include <boost/format.hpp>
#include <boost/filesystem.hpp>

#include "image_util.h"
#include "detect_points.h"
#include "compute_descriptors.h"

void readRegions( const char fileName[], uint32_t &numRegs, std::vector<ellipse> &regions ){
    
    std::ifstream fin( fileName );
    if (!fin.is_open()){
        regions.clear();
        numRegs= 0;
    }
    
    double temp_, x, y, a, b, c;
    fin>>temp_;
    fin>>numRegs;
    regions.resize( numRegs );
    for (uint32_t i=0; i<numRegs; ++i){
        fin>>x>>y>>a>>b>>c;
        regions[i].set(x,y,a,b,c);
    }
    
    fin.close();
    
}



void writeRegions( const char fileName[], std::vector<ellipse> const &regions ){
    
    std::ofstream fout( fileName );
    
    double x, y, a, b, c;
    fout<<"1.0\n";
    fout<<regions.size()<<"\n";
    for (uint32_t i=0; i<regions.size(); ++i){
        regions[i].get(x,y,a,b,c);
        fout<<x<<" "<<y<<" "<<a<<" "<<b<<" "<<c<<"\n";
    }
    
    fout.close();
    
}



void readRegsAndDescs( const char fileName[], uint32_t &numFeats, std::vector<ellipse> &regions, float **&descs ){
    
    double temp_, x, y, a, b, c;
    uint32_t numDims;
    
    std::ifstream fin( fileName );
    
    fin>>numDims>>numFeats;
    
    regions.resize( numFeats );
    uint32_t iDim;
    
    descs = new float*[numFeats];
    
    for (uint32_t i=0; i<numFeats; ++i){
        fin>>x>>y>>temp_>>a>>b>>c;
        regions[i].set(x,y,a,b,c);
        descs[i]= new float[numDims];
        for (iDim=0; iDim<numDims; ++iDim)
            fin>>descs[i][iDim];
    }
    
    fin.close();
}



void reg_KM_HessAff::getRegs(const char filename_jpeg[], uint32_t &numRegs, std::vector<ellipse> &regions ) {
    boost::filesystem::path tmp_dir = boost::filesystem::temp_directory_path() / "imcomp";
    tmp_dir = tmp_dir / "tmp";
    std::string region_filename = (tmp_dir / boost::filesystem::unique_path("imcomp_region_%%%%-%%%%-%%%%-%%%%.txt")).string();
    
    // form command line:
    // boost::format("detect_points_2.ln -i \"%s\" -hesaff -o \"%s\" > /dev/null") % fileName % tempRegsFn
    std::vector<std::string> args_;
    args_.push_back("detect_points_2.ln");
    args_.push_back("-i");
    args_.push_back(filename_jpeg);
    args_.push_back("-hesaff");
    args_.push_back("-o");
    args_.push_back(region_filename);
    
    // convert to char
    std::vector<char*> args;
    for (uint32_t i= 0; i<args_.size(); ++i){
        args_[i]+= '\0';
        args.push_back(&args_[i][0]);
    }
    
    // execute
    KM_detect_points::lib_main(args.size(),&args[0]);
        
    // read
    readRegions( region_filename.c_str(), numRegs, regions );
    
    // cleanup
    boost::filesystem::remove(region_filename);
}



void desc_KM_SIFT::getDescs(const char filename_jpeg[], std::vector<ellipse> &regions, uint32_t &numFeats, float **&descs ) {
    float scaleMulti = 1.732;
    bool upright = 0;

    boost::filesystem::path tmp_dir = boost::filesystem::temp_directory_path() / "imcomp";
    tmp_dir = tmp_dir / "tmp";
    std::string region_filename = (tmp_dir / boost::filesystem::unique_path("imcomp_regs_%%%%-%%%%-%%%%-%%%%.txt")).string();
    std::string descriptor_filename = (tmp_dir / boost::filesystem::unique_path("imcomp_desc_%%%%-%%%%-%%%%-%%%%.txt")).string();
        
    // write regions
    writeRegions(region_filename.c_str(), regions );
    
    // compute SIFT
    
    // form command line:
    // boost::format("compute_descriptors_2.ln -i \"%s\" -p1 \"%s\" -sift -o3 \"%s\" -scale-mult %.3f") % fileName % tempRegsFn % tempDescsFn % scaleMulti
    std::vector<std::string> args_;
    args_.push_back("compute_descriptors_2.ln");
    args_.push_back("-i");
    args_.push_back(filename_jpeg);
    args_.push_back("-p1");
    args_.push_back(region_filename);
    args_.push_back("-sift");
    args_.push_back("-o3");
    args_.push_back(descriptor_filename);
    args_.push_back("-scale-mult");
    args_.push_back( (boost::format("%.3f") % scaleMulti).str() );
    if (upright)
        args_.push_back("-noangle");
    
    // convert to char
    std::vector<char*> args;
    for (uint32_t i= 0; i<args_.size(); ++i){
        args_[i]+= '\0';
        args.push_back(&args_[i][0]);
    }
    
    // execute
    KM_compute_descriptors::lib_main(args.size(),&args[0]);

    // store descriptors in variable desc
    readRegsAndDescs(descriptor_filename.c_str(), numFeats, regions, descs );

    // cleanup temp. files
    boost::filesystem::remove(region_filename);
    boost::filesystem::remove(descriptor_filename);
}