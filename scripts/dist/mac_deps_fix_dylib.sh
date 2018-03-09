## By default, the imagemagick dylib have absolute path embedded in it
## This makes it difficult to ship these libraries with an installed
## This script removes those absolute paths
##
## Author: Abhishek Dutta <adutta@robots.ox.ac.uk>
## March 08, 2018

DYLIB_SRC="/Users/tlm/deps/clang/imcomp/lib/lib"
DYLIB_DST="/Users/tlm/deps/clang/imcomp/cpack_mac_deps"
#DYLIB_DST="/Users/tlm/dev/imcomp/bin"

# copy dependencies
cp /usr/local/opt/jpeg/lib/libjpeg.8.dylib "$DYLIB_SRC/"
cp /usr/local/opt/libpng/lib/libpng16.16.dylib "$DYLIB_SRC/"
cp /usr/local/opt/xz/lib/liblzma.5.dylib "$DYLIB_SRC/"
cp /Users/tlm/deps/clang/imcomp/tmp_libsrc/vlfeat-0.9.21/bin/maci64/libvl.dylib "$DYLIB_SRC/"
cp /Users/tlm/deps/clang/imcomp/tmp_libsrc/vlfeat-0.9.21/bin/maci64/libomp.dylib "$DYLIB_SRC/"

# remove absolute path from id of dependencies
install_name_tool -id "libjpeg.8.dylib" "$DYLIB_SRC"/libjpeg.8.dylib
install_name_tool -id "libpng16.16.dylib" "$DYLIB_SRC"/libpng16.16.dylib
install_name_tool -id "liblzma.5.dylib" "$DYLIB_SRC"/liblzma.5.dylib

##
## Magick++
##
# remove absolute path ids from ImageMagick libraries
install_name_tool -id "libMagick++-6.Q8.8.dylib" "$DYLIB_SRC"/libMagick++-6.Q8.8.dylib
# update dependency so that the absolute path is removed
install_name_tool -change "$DYLIB_SRC"/libMagickCore-6.Q8.5.dylib "@rpath/libMagickCore-6.Q8.5.dylib" "$DYLIB_SRC"/libMagick++-6.Q8.dylib
install_name_tool -change "$DYLIB_SRC"/libMagickWand-6.Q8.5.dylib "@rpath/libMagickWand-6.Q8.5.dylib" "$DYLIB_SRC"/libMagick++-6.Q8.dylib
# remove absolute path from dependencies in Magick
install_name_tool -change /usr/local/opt/jpeg/lib/libjpeg.8.dylib "@rpath/libjpeg.8.dylib" "$DYLIB_SRC"/libMagick++-6.Q8.dylib
install_name_tool -change /usr/local/opt/libpng/lib/libpng16.16.dylib "@rpath/libpng16.16.dylib" "$DYLIB_SRC"/libMagick++-6.Q8.dylib
install_name_tool -change /usr/local/opt/xz/lib/liblzma.5.dylib "@rpath/liblzma.5.dylib" "$DYLIB_SRC"/libMagick++-6.Q8.dylib

##
## MagickCore
##
# remove absolute path ids from ImageMagick libraries
install_name_tool -id libMagickCore-6.Q8.dylib "$DYLIB_SRC"/libMagickCore-6.Q8.dylib
# remove absolute path from dependencies in Magick
install_name_tool -change /usr/local/opt/jpeg/lib/libjpeg.8.dylib "@rpath/libjpeg.8.dylib" "$DYLIB_SRC"/libMagickCore-6.Q8.dylib
install_name_tool -change /usr/local/opt/libpng/lib/libpng16.16.dylib "@rpath/libpng16.16.dylib" "$DYLIB_SRC"/libMagickCore-6.Q8.dylib
install_name_tool -change /usr/local/opt/xz/lib/liblzma.5.dylib "@rpath/liblzma.5.dylib" "$DYLIB_SRC"/libMagickCore-6.Q8.dylib

##
## MagickWand
##
# remove absolute path ids from ImageMagick libraries
install_name_tool -id libMagickWand-6.Q8.dylib "$DYLIB_SRC"/libMagickWand-6.Q8.dylib
# remove absolute path from dependencies in Magick
install_name_tool -change "$DYLIB_SRC"/libMagickCore-6.Q8.5.dylib "@rpath/libMagickCore-6.Q8.5.dylib" "$DYLIB_SRC"/libMagickWand-6.Q8.dylib
install_name_tool -change /usr/local/opt/jpeg/lib/libjpeg.8.dylib "@rpath/libjpeg.8.dylib" "$DYLIB_SRC"/libMagickWand-6.Q8.dylib
install_name_tool -change /usr/local/opt/libpng/lib/libpng16.16.dylib "@rpath/libpng16.16.dylib" "$DYLIB_SRC"/libMagickWand-6.Q8.dylib
install_name_tool -change /usr/local/opt/xz/lib/liblzma.5.dylib "@rpath/liblzma.5.dylib" "$DYLIB_SRC"/libMagickWand-6.Q8.dylib

##
## Boost libraries
##
#install_name_tool -change "libboost_system.dylib" "@rpath/libboost_system.dylib" "$DYLIB_SRC"/libboost_chrono.dylib
#install_name_tool -change "libboost_system.dylib" "@rpath/libboost_system.dylib" "$DYLIB_SRC"/libboost_filesystem.dylib
#install_name_tool -change "libboost_system.dylib" "@rpath/libboost_system.dylib" "$DYLIB_SRC"/libboost_thread.dylib
#install_name_tool -change "libboost_chrono.dylib" "@rpath/libboost_chrono.dylib" "$DYLIB_SRC"/libboost_timer.dylib
#install_name_tool -change "libboost_system.dylib" "@rpath/libboost_system.dylib" "$DYLIB_SRC"/libboost_timer.dylib

##
## vlfeat
##
install_name_tool -change "@loader_path/libvl.dylib" "@rpath/libvl.dylib" "$DYLIB_SRC"/libboost_timer.dylib
# todo
install_name_tool -change "@loader_path/libomp.dylib" "@rpath/libomp.dylib" "$DYLIB_SRC"/libboost_timer.dylib

cp -f "$DYLIB_SRC/libMagick++-6.Q8.8.dylib" "$DYLIB_DST/"
cp -f "$DYLIB_SRC/libMagickCore-6.Q8.5.dylib" "$DYLIB_DST/"
cp -f "$DYLIB_SRC/libMagickWand-6.Q8.5.dylib" "$DYLIB_DST/"
cp -f "$DYLIB_SRC/libjpeg.8.dylib" "$DYLIB_DST/"
cp -f "$DYLIB_SRC/libpng16.16.dylib" "$DYLIB_DST/"
cp -f "$DYLIB_SRC/liblzma.5.dylib" "$DYLIB_DST/"

#cp -f "$DYLIB_SRC/libboost_atomic.dylib" "$DYLIB_DST/"
#cp -f "$DYLIB_SRC/libboost_chrono.dylib" "$DYLIB_DST/"
#cp -f "$DYLIB_SRC/libboost_date_time.dylib" "$DYLIB_DST/"
#cp -f "$DYLIB_SRC/libboost_filesystem.dylib" "$DYLIB_DST/"
#cp -f "$DYLIB_SRC/libboost_system.dylib" "$DYLIB_DST/"
#cp -f "$DYLIB_SRC/libboost_thread.dylib" "$DYLIB_DST/"
#cp -f "$DYLIB_SRC/libboost_timer.dylib" "$DYLIB_DST/"

cp -f "$DYLIB_SRC/libvl.dylib" "$DYLIB_DST/"
cp -f "$DYLIB_SRC/libomp.dylib" "$DYLIB_DST/"
