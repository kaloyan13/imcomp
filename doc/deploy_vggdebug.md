#!/bin/sh

DEP_BASEDIR="/ssd/adutta/build_deps/"
CMAKE_BIN_DIR="${DEP_BASEDIR}lib/cmake/bin"
IMAGEMAGICK_LIBDIR="${DEP_BASEDIR}lib/imagemagick_q8/"
BOOST_LIBDIR="${DEP_BASEDIR}lib/boost/"
EIGEN_LIBDIR="${DEP_BASEDIR}lib/eigen"
export CMAKE_PREFIX_PATH=$IMAGEMAGICK_LIBDIR:$BOOST_LIBDIR:$EIGEN_LIBDIR

export CMAKE_PREFIX_PATH=/ssd/adutta/build_deps/lib/imagemagick_q8/:/ssd/adutta/build_deps/lib/boost/:/ssd/adutta/build_deps/lib/eigen/
cd /data/adutta/vggdemo/traherne/imcomp
mkdir build
cd build
/ssd/adutta/build_deps/lib/cmake/bin/cmake -DCMAKE_BUILD_TYPE=Release ../
make -j8

bin/imcomp_server 0.0.0.0 11010 4 /ssd/adutta/data/vggdemo/traherne
