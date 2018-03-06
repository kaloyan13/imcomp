#!/bin/sh
#
# Script to build MacOS releases
# Author: Abhishek Dutta <adutta@robots.ox.ac.uk>
# March 06, 2018

RELEASE_PACKAGE_NAME="Traherne"
RELEASE_PACKAGE_DESCRIPTION="Traherne Digital Collator"
IMCOMP_SERVER_VERSION_PATCH="3"

BUILD_BASE_FOLDER="/Users/tlm/builds/imcomp"
IMCOMP_SOURCE_PATH="/Users/tlm/dev/imcomp"
IMCOMP_DEPS_DIR="/Users/tlm/deps/imcomp/lib"

(cd $BUILD_BASE_FOLDER;
  cmake DCMAKE_C_COMPILER=/usr/local/bin/gcc-6 -DCMAKE_CXX_COMPILER=/usr/local/bin/g++-6 -DCMAKE_BUILD_TYPE=Release -DIMCOMP_SERVER_VERSION_PATCH=$IMCOMP_SERVER_VERSION_PATCH -DCMAKE_PREFIX_PATH=$IMCOMP_DEPS_DIR $IMCOMP_SOURCE_PATH;
)
