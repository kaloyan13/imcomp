#!/bin/sh

curdir=`pwd`
SRC_DIR="${curdir}/src"
INC_DIR="${curdir}/include"

if [ ! -d $SRC_DIR ] && [ ! -d $INC_DIR ]; then
  echo "Run this script from the root folder and not from inside the script/ folder";
  exit 1;
fi

#rm -fr "${curdir}/build"
mkdir -p "${curdir}/build"
cd "${curdir}/build"
#cmake ../
cmake ../ && make -j8

#rm -fr /tmp/imcomp
"${curdir}/bin/imcomp_server" 0.0.0.0 9971 4 /tmp/imcomp


