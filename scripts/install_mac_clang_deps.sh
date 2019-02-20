# Abhishek Dutta <adutta@robots.ox.ac.uk>
# March 06, 2018
DEP_BASEDIR=$1

if [ -d "$DEP_BASEDIR" ]; then
    echo "Dependency directory already exists. Re-using this directory!"
else
    echo "Usage: $0 DEPENDENCY_BASE_DIR"
    exit
fi

TMP_BASEDIR=$DEP_BASEDIR"tmp_libsrc/"
LIBDIR=$DEP_BASEDIR"lib/"

if [ ! -d "$LIBDIR" ]; then
    mkdir $LIBDIR
fi
if [ ! -d "$TMP_BASEDIR" ]; then
    mkdir $TMP_BASEDIR
fi

#
# Install ImageMagick
#
if [ -d "$LIBDIR/include/ImageMagick-6" ]; then
    echo "Skipping Imagemagick library installation as it already exists"
else
    cd $TMP_BASEDIR
    wget -O ImageMagick-6.9.9-36.tar.gz --no-clobber https://codeload.github.com/ImageMagick/ImageMagick/tar.gz/6.9.9-36
    tar -zxvf ImageMagick-6.9.9-36.tar.gz
    cd ImageMagick-6.9.9-36
    #./configure --prefix=$LIBDIR --enable-hdri=no --with-quantum-depth=8 --with-x=no --without-perl --disable-deprecated
    #./configure --prefix=/Users/tlm/deps/clang/imcomp/lib --disable-installed --with-quantum-depth=8 --disable-hdri --without-modules --without-perl --without-freetype --without-dps --without-fpx --without-jbig --without-tiff --without-wmf --without-fontpath --without-gs-font-dir --without-windows-font-dir --without-x --without-fontconfig --without-pango --enable-shared=no --enable-static=yes --disable-dependency-tracking
    ./configure --prefix=/Users/tlm/deps/clang/imcomp/lib --disable-installed --with-quantum-depth=8 --disable-hdri --without-perl --without-freetype --without-dps --without-fpx --without-jbig --with-tiff --without-wmf --without-fontpath --without-gs-font-dir --without-windows-font-dir --without-x --without-fontconfig --without-pango --enable-shared=yes --enable-static=no --disable-dependency-tracking --enable-zero-configuration
    make -j 8
    make install
fi

#
# Install Boost
#
if [ -d "$LIBDIR/include/boost" ]; then
    echo "Skipping Boost library installation as it already exists"
else
    cd $TMP_BASEDIR
    wget --no-clobber https://netcologne.dl.sourceforge.net/project/boost/boost/1.64.0/boost_1_64_0.tar.gz
    tar -zxvf boost_1_64_0.tar.gz
    cd boost_1_64_0
    ./bootstrap.sh --prefix=$LIBDIR --with-libraries=filesystem,system,thread,date_time,chrono,atomic,timer
    ./b2 --with-filesystem --with-system --with-thread --with-date_time --with-chrono --with-atomic --with-timer variant=release threading=multi link=static runtime-link=static install
fi

#
# Install Eigen
#
if [ -d "$TMP_BASEDIR/Eigen-3.3.4" ]; then
    echo "Skipping Eigen library installation as it already exists"
else
    cd $TMP_BASEDIR
    wget -O Eigen-3.3.4.tar.gz --no-clobber http://bitbucket.org/eigen/eigen/get/3.3.4.tar.gz
    tar -zxvf Eigen-3.3.4.tar.gz
    mv eigen-* Eigen-3.3.4
    cd Eigen-3.3.4
    mkdir build
    cd build
    cmake  -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$LIBDIR" -DBOOST_ROOT="$LIBDIR" ../
    make j 8
fi

#
# Install vlfeat
#
if [ -d "$TMP_BASEDIR/Eigen-3.3.4" ]; then
    echo "Skipping Eigen library installation as it already exists"
else
    cd $TMP_BASEDIR
    wget -O vlfeat-0.9.21.tar.gz http://www.vlfeat.org/download/vlfeat-0.9.21-bin.tar.gz
    tar -zxvf vlfeat-0.9.21.tar.gz
    ## vlfeat already contains precompile libraries, therefore no need to compile
fi
