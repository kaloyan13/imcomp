#!/bin/sh

if [ "$#" -ne 1 ] || ! [ -d "$1" ]; then
  echo "Usage: $0 DEPENDENCY_FILES_LOCATION/" >&2
  exit 1
fi

# $1
# |- lib  
# |- tmp_libsrc
DEP_BASEDIR=$1
DEP_LIBDIR="${DEP_BASEDIR}lib/"
DEP_SRCDIR="${DEP_BASEDIR}tmp_libsrc"

#
# Install ImageMagick
#
IMAGEMAGICK_LIBDIR=$DEP_LIBDIR"imagemagick_q8/"
if [ -d "$IMAGEMAGICK_LIBDIR" ]; then
    echo "Skipping ImageMagick library as it already exists at "$IMAGEMAGICK_LIBDIR
else
    echo "Installing imagemagick at "$IMAGEMAGICK_LIBDIR
    cd $DEP_SRCDIR
    #wget https://www.imagemagick.org/download/releases/ImageMagick-6.9.8-4.tar.gz
    #wget -O ImageMagick-6.9.8-4.tar.gz http://git.imagemagick.org/repos/ImageMagick/repository/archive.tar.gz?ref=6.9.8-4
    #wget -O ImageMagick-7.0.5-6.tar.gz http://git.imagemagick.org/repos/ImageMagick/repository/archive.tar.gz?ref=7.0.5-6
    #wget -O ImageMagick-6.9.8-8.tar.gz http://git.imagemagick.org/repos/ImageMagick/repository/archive.tar.gz?ref=6.9.8-8
    #tar -zxvf ImageMagick-6.9.8-8.tar.gz
    cd ImageMagick-6.9.8-8
    ./configure --prefix=$IMAGEMAGICK_LIBDIR -enable-hdri=no --with-quantum-depth=8 --disable-dependency-tracking --with-x=yes --without-perl
    make -j8
    make install
fi

#
# Install vlfeat
#
VLFEAT_LIBDIR=$DEP_LIBDIR"vlfeat/"
if [ -d "$VLFEAT_LIBDIR" ]; then
    echo "Skipping VLFEAT library as it already exists at "$VLFEAT_LIBDIR
else
    echo "Installing VLFEAT at "$VLFEAT_LIBDIR
    cd $DEP_SRCDIR
    wget http://www.vlfeat.org/download/vlfeat-0.9.21-bin.tar.gz
    tar -zxvf vlfeat-0.9.21-bin.tar.gz 
    cd vlfeat-0.9.21/
    ./configure --prefix=$IMAGEMAGICK_LIBDIR -enable-hdri=no --with-quantum-depth=8 --disable-dependency-tracking --with-x=yes --without-perl
    make -j8
    make install
fi

#
# Install vlfeat
#
EIGEN_LIBDIR=$DEP_LIBDIR"eigen/"
if [ -d "$EIGEN_LIBDIR" ]; then
    echo "Skipping EIGEN library as it already exists at "$EIGEN_LIBDIR
else
    echo "Installing EIGEN at "$EIGEN_LIBDIR
    cd $DEP_SRCDIR
    wget -O eigen-3.3.4.tar.gz http://bitbucket.org/eigen/eigen/get/3.3.4.tar.gz
    tar -zxvf eigen-3.3.4.tar.gz
    cd eigen-3.3.4/
    mkdir build
    cd build
    /ssd/adutta/build_deps/lib/cmake/bin/cmake -DCMAKE_INSTALL_PREFIX=/ssd/adutta/build_deps/lib/eigen ../
    make install
fi

