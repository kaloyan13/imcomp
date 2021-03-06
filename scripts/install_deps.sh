#!/bin/sh

## check dependency location provided by user
if [ "$#" -ne 1 ]; then
  echo "Usage: $0 [FOLDER TO STORE DEPENDENCIES]" >&2
  exit 1
fi
if ! [ -e "$1" ]; then
  echo "$1 not found" >&2
  exit 1
fi
if ! [ -d "$1" ]; then
  echo "$1 not a directory" >&2
  exit 1
fi

DEPDIR=$1
DEPSRC="${DEPDIR}/_tmp_libsrc"
if ! [ -d "${DEPSRC}" ]; then
  mkdir "${DEPSRC}"
fi

#sudo apt install libssl-dev # required by cmake

## cmake
if ! [ -f "${DEPDIR}/bin/cmake" ]; then
  cd $DEPSRC && wget https://github.com/Kitware/CMake/releases/download/v3.18.0/cmake-3.18.0.tar.gz && tar -zxvf cmake-3.18.0.tar.gz && cd cmake-3.18.0 && ./configure --prefix=$DEPDIR && make -j 16 && make install
fi

## boost
if ! [ -d "${DEPDIR}/include/boost" ]; then
  cd $DEPSRC && wget https://dl.bintray.com/boostorg/release/1.73.0/source/boost_1_73_0.tar.gz && tar -zxvf boost_1_73_0.tar.gz && cd boost_1_73_0 && ./bootstrap.sh --prefix=$DEPDIR --with-toolset=gcc --with-libraries=filesystem,system,thread,atomic && ./b2 --with-filesystem --with-system --with-thread --with-atomic variant=release threading=multi toolset=gcc install
fi

# imagemagick
if ! [ -d "${DEPDIR}/include/ImageMagick-6" ]; then
  cd $DEPSRC && wget -O ImageMagick6-6.9.11-22.tar.gz https://github.com/ImageMagick/ImageMagick6/archive/6.9.11-22.tar.gz && tar -zxvf ImageMagick6-6.9.11-22.tar.gz && cd ImageMagick6-6.9.11-22 && ./configure --prefix=$DEPDIR -enable-hdri=no --with-quantum-depth=8 --disable-dependency-tracking --with-x=no --without-perl && make -j 16 && make install
fi

# eigen
if ! [ -d "${DEPDIR}/include/eigen3/Eigen" ]; then
  cd $DEPSRC && wget -O eigen-3.3.7.tar.gz https://gitlab.com/libeigen/eigen/-/archive/3.3.7/eigen-3.3.7.tar.gz && tar -zxvf eigen-3.3.7.tar.gz && cd eigen-3.3.7/ && mkdir cmake_build && cd cmake_build && $DEPDIR"/bin/cmake" -DCMAKE_INSTALL_PREFIX=$DEPDIR ../ && make -j8 && make install
fi

# vlfeat
if ! [ -d "${DEPDIR}/include/vl" ]; then
  cd $DEPSRC && wget http://www.vlfeat.org/download/vlfeat-0.9.21-bin.tar.gz && tar -zxvf vlfeat-0.9.21-bin.tar.gz && cd vlfeat-0.9.21 && make -j16 && cp "${DEPSRC}/vlfeat-0.9.21/bin/glnxa64/libvl.so" "${DEPDIR}/lib/libvl.so" && mkdir "${DEPDIR}/include/vl" && cp -fr $DEPSRC/vlfeat-0.9.21/vl/*.* "${DEPDIR}/include/vl/"
fi

echo "****************************************************************"
echo "All dependencies downloaded, compiled and installed to ${DEPDIR}"
