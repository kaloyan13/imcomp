# Compiling Image Comparator

## Install Dependencies
 * cmake
```
cd /home/tlm/deps/imcomp/tmp_libsrc
wget https://cmake.org/files/v3.10/cmake-3.10.2.tar.gz
tar -zxvf cmake-3.10.2.tar.gz
cd cmake-3.10.2/
./configure --parallel=8 --prefix=/home/tlm/deps/imcomp/lib/
make -j 8
make install
```

 * Eigen
```
# eigen dependecies
sudo apt-get install libsuitesparse-dev libsparsehash-dev libsuperlu-dev

cd /home/tlm/deps/imcomp/tmp_libsrc
wget -O eigen-3.3.4.tar.gz http://bitbucket.org/eigen/eigen/get/3.3.4.tar.gz
tar -zxvf eigen-3.3.4.tar.gz
cd eigen-eigen-5a0156e40feb/
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=/home/tlm/deps/imcomp/lib ../
make install
```

 * vlfeat
```
cd /home/tlm/deps/imcomp/tmp_libsrc
wget http://www.vlfeat.org/download/vlfeat-0.9.20-bin.tar.gz
tar -zxvf vlfeat-0.9.20-bin.tar.gz 
cd vlfeat-0.9.20
make -j 8
```

 * ImageMagick
```
cd /home/tlm/deps/imcomp/tmp_libsrc
wget https://www.imagemagick.org/download/ImageMagick-6.9.9-34.tar.gz
tar -zxvf ImageMagick-6.9.9-34.tar.gz
cd ImageMagick-6.9.9-34/
./configure --prefix=/home/tlm/deps/imcomp/lib -enable-hdri=no --with-quantum-depth=8 --disable-dependency-tracking --with-x=yes --without-perl
make -j 8 && make install
```

 * Boost
```
cd /home/tlm/deps/imcomp/tmp_libsrc
wget https://dl.bintray.com/boostorg/release/1.64.0/source/boost_1_64_0.tar.gz
tar -zxvf boost_1_64_0.tar.gz
./bootstrap.sh --prefix=/home/tlm/deps/imcomp/lib --with-libraries=filesystem,system,thread,atomic,chrono,date_time
./b2 -j 8 install
```


## Compile Image Comparator Source Code
```
cd /home/tlm/dev/imcomp
mkdir build
/home/tlm/deps/imcomp/lib/bin/cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=/home/tlm/deps/imcomp/lib -DVLFEAT_LIB=/home/tlm/deps/imcomp/tmp_libsrc/vlfeat-0.9.20/bin/glnxa64/libvl.so -DVLFEAT_INCLUDE_DIR=/home/tlm/deps/imcomp/tmp_libsrc/vlfeat-0.9.20 ../
```
