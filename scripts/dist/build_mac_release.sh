#!/bin/sh
#
# Script to build MacOS releases
# Author: Abhishek Dutta <adutta@robots.ox.ac.uk>
# March 06, 2018

RELEASE_PACKAGE_NAME="Traherne"
RELEASE_PACKAGE_DESCRIPTION="Traherne Digital Collator"
IMCOMP_SERVER_VERSION_PATCH="5"

BUILD_BASE_FOLDER="/Users/tlm/builds/imcomp"
IMCOMP_SOURCE_PATH="/Users/tlm/dev/imcomp"
IMCOMP_DEPS_DIR="/Users/tlm/deps/clang/imcomp/lib"

#if [ -d "/Users/tlm/builds/imcomp" ]; then
  #rm -fr /Users/tlm/builds/imcomp
#fi
mkdir -p /Users/tlm/builds/imcomp

(cd $BUILD_BASE_FOLDER;
 cmake -DCMAKE_VERBOSE_MAKEFILE:BOOL=YES -DCMAKE_BUILD_TYPE=Release -DRELEASE_PACKAGE_NAME=$RELEASE_PACKAGE_NAME -DRELEASE_PACKAGE_DESCRIPTION=$RELEASE_PACKAGE_DESCRIPTION -DIMCOMP_SERVER_VERSION_PATCH=$IMCOMP_SERVER_VERSION_PATCH -DCMAKE_PREFIX_PATH=$IMCOMP_DEPS_DIR $IMCOMP_SOURCE_PATH ;
 make -j 8 ;
 make package
)
