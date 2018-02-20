# Compiling IMCOMP in Windows

## Compile Dependencies
* install git for windows from https://git-scm.com/download/win

* git clone git@gitlab.com:vgg/imcomp.git

* Install dependencies
 * install boost library
	- https://sourceforge.net/projects/boost/files/boost-binaries/1.65.1/boost_1_65_1-msvc-14.1-64.exe/download
    - Update imcomp/CMakeLists.txt file to point to boost install location
 * install imagemagick library
	- only 32 bit library is found by cmake : ImageMagick-6.9.9-33-Q16-x86-dll.exe and ImageMagick-6.9.9-33-Q8-x86-dll.exe
	- why?
 * install vlfeat
  - http://www.vlfeat.org/download/vlfeat-0.9.20-bin.tar.gz
  - update "C:\Users\tlm\deps\vlfeat\vlfeat-0.9.20\make\nmake_helper.mak"
 * compile eigen3
  - download from http://bitbucket.org/eigen/eigen/get/3.3.4.zip
  - mkdir build, cd build
  - "c:\Program Files\CMake\bin\cmake.exe" -G "Visual Studio 15 2017" "C:\Users\tlm\deps\eigen\eigen-eigen-5a0156e40feb\eigen-eigen-5a0156e40feb" -DCMAKE_GENERATOR_PLATFORM=x64 -DCMAKE_BUILD_TYPE=Release 

* Download and install cmake from https://cmake.org/download/ and run cmake-gui tool
	- help
		- https://dmerej.info/blog/post/cmake-visual-studio-and-the-command-line/
	- cd C:\Users\tlm\build\imcomp
	- "c:\Program Files\CMake\bin\cmake.exe" -G "Visual Studio 15 2017" "C:\Users\tlm\dev\imcomp" -DCMAKE_GENERATOR_PLATFORM=x86 ## @todo: setup configuration
  - "c:\Program Files\CMake\bin\cmake.exe" --version
  - 
	- devenv /build Debug imcomp_server.sln

## Compile IMCOMP
```
mkdir build
cd build
"c:\Program Files\CMake\bin\cmake.exe" -G "Visual Studio 15 2017" "C:\Users\tlm\dev\imcomp" -DCMAKE_GENERATOR_PLATFORM=x64 -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="C:\Users\tlm\deps\lib\include\eigen3" -DVLFEAT_LIB="C:\Users\tlm\deps\lib\lib\vl.dll" -DVLFEAT_INCLUDE_DIR="C:\Users\tlm\deps\lib\include"
cls&msbuild ALL_BUILD.vcxproj /p:configuration=Release
```


## Code Updates
 * Update src/http_server/http_request.cc
```
http_request: ((size_t) payload_.tellg()) == content_length
```

 * Update vl/generic.h
```
// added by Abhishek Dutta (20 Feb. 2018) to compile in Windows Visual Studio 2017
#if defined(VL_COMPILER_MSC) & ! defined(__DOXYGEN__)
#  undef snprintf
#endif
```

 * Update src/imreg_sift/imreg_sift.cc
```
/* allocate buffer */
  data  = (vl_uint8*) malloc( vl_pgm_get_npixels(&pim) * vl_pgm_get_bpp(&pim) * sizeof(vl_uint8) ) ;
  fdata = (vl_sift_pix*) malloc( vl_pgm_get_npixels(&pim) * vl_pgm_get_bpp(&pim) * sizeof(vl_sift_pix) ) ;
```