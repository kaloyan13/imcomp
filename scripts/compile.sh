#!/bin/sh

curdir=`pwd`
SRC_DIR="${curdir}/src"
INC_DIR="${curdir}/include"

if [ ! -d $SRC_DIR ] && [ ! -d $INC_DIR ]; then
  echo "Run this script from the root folder and not from inside the script/ folder";
  exit 1;
fi

#rm -fr "${curdir}/build"
#mkdir -p "${curdir}/build"
cd "${curdir}/build"
/home/tlm/deps/imcomp/lib/bin/cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH=/home/tlm/deps/imcomp/lib -DVLFEAT_LIB=/home/tlm/deps/imcomp/tmp_libsrc/vlfeat-0.9.20/bin/glnxa64/libvl.so -DVLFEAT_INCLUDE_DIR=/home/tlm/deps/imcomp/tmp_libsrc/vlfeat-0.9.20 ../
#/home/tlm/deps/imcomp/lib/bin/cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=/home/tlm/deps/imcomp/lib -DVLFEAT_LIB=/home/tlm/deps/imcomp/tmp_libsrc/vlfeat-0.9.20/bin/glnxa64/libvl.so -DVLFEAT_INCLUDE_DIR=/home/tlm/deps/imcomp/tmp_libsrc/vlfeat-0.9.20 ../

make -j 8

#rm -fr /tmp/imcomp
#"${curdir}/bin/imcomp_server" 0.0.0.0 9973 4 /home/tlm/dev/imcomp/asset/imcomp /tmp/imcomp


