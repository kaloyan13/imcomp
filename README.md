# Image Comparator

Image comparator is a standalone and lightweight tool to compare a pair of images.
It is an open source project developed at the [Visual Geometry Group](http://www.robots.ox.ac.uk/~vgg/) and
released under the BSD-2 clause license. For more details, see [http://www.robots.ox.ac.uk/~vgg/software/imcomp/].
For more details, see http://www.robots.ox.ac.uk/~vgg/software/traherne/

## Compiling from source
This software builds on the following open source software libraries:
 * [Boost C++ Libraries](http://www.boost.org/)
 * [ImageMagick Magick++ Library](https://www.imagemagick.org/script/magick++.php)
 * [VLFEAT C API](http://www.vlfeat.org/)
 * [Eigen](http://eigen.tuxfamily.org/index.php?title=Main_Page)
You must install these libraries prior to compiling this software as follows:

```
# Fedora 35 packages
dnf install boost-devel ImageMagick-devel ImageMagick-c++-devel eigen3-devel octave-devel

# Ubuntu 20 packages
apt-get install make gcc g++ libboost-dev libboost-filesystem-dev libboost-thread-dev imagemagick cmake libeigen3-dev libmagick++-dev

# install VLFEAT
wget -P/tmp https://www.vlfeat.org/download/vlfeat-0.9.21-bin.tar.gz
tar -xf vlfeat-0.9.21-bin.tar.gz
cd vlfeat-0.9.21
make

mkdir /usr/include/vl
cp vl/*.h /usr/include/vl
cp bin/glnxa64/libvl.so /usr/lib/

cd $IMCOMP_SOURCE_FOLDER # replace this with the folder that contains source code
mkdir build
cd build
cmake ../ -DVLFEAT_INCLUDE_DIR=/usr/include -DVLFEAT_LIB=/usr/lib/libvl.so
make -j 8

# Link or create the library folder in asset
  ln -s YOUR_LIB_FOLDER $IMCOMP_SOURCE_FOLDER/asset/imcomp/library
  ls library/
A.jpg  B.jpg

# to run the imcomp_server
../bin/imcomp_server 0.0.0.0 9972 4 $IMCOMP_SOURCE_FOLDER/asset /tmp/imcomp

# To use this applicaton, visit http://localhost:9972/imcomp/traherne in your web browser
# To use library compare, visit http://localhost:9972/imcomp/index.html?images=A.jpg,B.jpg in your web browser
```


## Contact
  * [Abhishek Dutta](adutta@robots.ox.ac.uk)
  * Project started on : Jan. 04, 2017
