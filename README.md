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
cd $IMCOMP_SOURCE_FOLDER # replace this with the folder that contains source code
mkdir build
cd build
cmake ../
make -j 8

# to run the imcomp_server
../bin/imcomp_server 0.0.0.0 9972 4 $IMCOMP_SOURCE_FOLDER/asset /tmp/imcomp

# To use this applicaton, visit http://localhost:9972/imcomp/traherne in your web browser
```


## Contact
  * [Abhishek Dutta](adutta@robots.ox.ac.uk)
  * Project started on : Jan. 04, 2017
