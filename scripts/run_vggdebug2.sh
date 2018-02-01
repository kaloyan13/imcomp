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
/ssd/adutta/build_deps/lib/cmake/bin/cmake -DCMAKE_BUILD_TYPE=Release ../ ../ && make -j8

rm -fr /tmp/imcomp/
rm -fr /ssd/adutta/data/vggdemo/traherne/imcomp/
mkdir /ssd/adutta/data/vggdemo/traherne/imcomp/
"${curdir}/bin/imcomp_server" 0.0.0.0 11010 8 /ssd/adutta/data/vggdemo/traherne/imcomp/


