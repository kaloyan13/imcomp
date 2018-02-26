# Compiling IMCOMP in Windows

## Compile Dependencies
 * references
   - http://cpprocks.com/using-cmake-to-build-a-cross-platform-project-with-a-boost-dependency/
 * install git for windows from https://git-scm.com/download/win
 * git clone git@gitlab.com:vgg/imcomp.git

 * compile imagemagick
  - see https://www.imagemagick.org/script/advanced-windows-installation.php
  - download https://www.imagemagick.org/download/windows/ImageMagick-6.9.9-35.zip
  - build without X11 support
  - using Visual Studio 2017, open "C:\Users\tlm\deps\imagemagick\ImageMagick-6.9.9-35\ImageMagick-6.9.9-35\VisualMagick\configure\configure.sln"
  - install the required VS2017 components
  - right click solution and click "Retarget solution" to make it usable in VS2017
  - build to produce configure.exe
  - create VisualDynamicMT.sln by running "C:\Users\tlm\deps\imagemagick\ImageMagick-6.9.9-35\ImageMagick-6.9.9-35\VisualMagick\configure\configure.exe"
    * 64 bit
    * no OpenCL
    * YES OpenMP
    * Q8
    * DLL Multi-threaded
    * NO zero config
    * NO registry or hard coded
  - For the 64-bit build, you will also need to disable X11 support. Edit magick-config.h and undefine the MAGICKCORE_X11_DELEGATE define.
    * update "C:\Users\tlm\deps\win_x64\imagemagick\ImageMagick-6.9.9-35\ImageMagick\magick\magick-config.h"
```
#undef MAGICKCORE_X11_DELEGATE
#define ProvideDllMain
```

  - using Visual Studio 2017, open "C:\Users\tlm\deps\imagemagick\ImageMagick-6.9.9-35\ImageMagick-6.9.9-35\VisualMagick\VisualDynamicMT.sln"
    * Clean , Build

 * install boost library
	- https://sourceforge.net/projects/boost/files/boost-binaries/1.65.1/boost_1_65_1-msvc-14.1-64.exe/download
  - Update imcomp/CMakeLists.txt file to point to boost install location
  - compile only the optimized version of library (not the debug).
 
 * install vlfeat
  - http://www.vlfeat.org/download/vlfeat-0.9.21-bin.tar.gz
  - only win_x64 version of precompiled library is available
  - how to get win_x86?
  
 * compile eigen3
  - download from http://bitbucket.org/eigen/eigen/get/3.3.4.zip
  - mkdir build, cd build
  - "c:\Program Files\CMake\bin\cmake.exe" -G "Visual Studio 15 2017" "C:\Users\tlm\deps\win_x64\eigen\eigen-eigen-5a0156e40feb" -DCMAKE_GENERATOR_PLATFORM=x64 -DCMAKE_BUILD_TYPE=Release 

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
## 64 bit
cd Users\tlm\dev\imcomp\build\win_x64 
"c:\Program Files\CMake\bin\cmake.exe" -G "Visual Studio 15 2017" "C:\Users\tlm\dev\imcomp" -DCMAKE_GENERATOR_PLATFORM=x64 -DCMAKE_BUILD_TYPE=Release -DEIGEN_DIR="C:\Users\tlm\deps\win_x64\eigen\eigen-eigen-5a0156e40feb\build" -DVLFEAT_LIB="C:\Users\tlm\deps\win_x64\lib\lib\vl.lib" -DVLFEAT_INCLUDE_DIR="C:\Users\tlm\deps\win_x64\lib\include" -DBOOST_ROOT="C:\Users\tlm\deps\win_x64\boost\boost_1_65_1"
cls&msbuild ALL_BUILD.vcxproj /p:configuration=Release
msbuild ALL_BUILD.vcxproj /maxcpucount:8 -v:minimal -p:PreferredToolArchitecture=x64 /nologo /verbosity:quiet 
msbuild ALL_BUILD.vcxproj /maxcpucount:8 -v:minimal -p:PreferredToolArchitecture=x64 /nologo /p:configuration=Release

## 32 bit
"c:\Program Files\CMake\bin\cmake.exe" -G "Visual Studio 15 2017" "C:\Users\tlm\dev\imcomp" -DCMAKE_GENERATOR_PLATFORM=x86 -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="C:\Users\tlm\deps\lib\include\eigen3" -DVLFEAT_LIB="C:\Users\tlm\deps\lib\lib\vl.lib" -DVLFEAT_INCLUDE_DIR="C:\Users\tlm\deps\lib\include" -DBOOST_ROOT="C:/Users/tlm/deps/boost_1_65_1"
cls&msbuild ALL_BUILD.vcxproj /p:configuration=Release

```

## Executing IMCOMP
```
set MAGICK_HOME="C:\Users\tlm\dev\imcomp\bin\Release"
set MAGICK_CONFIGURE_PATH="C:\Users\tlm\dev\imcomp\bin\Release"
set MAGICK_DEBUG=True
set MAGICK_CODER_MODULE_PATH="C:\Users\tlm\dev\imcomp\bin\Release"
set MAGICK_CODER_FILTER_PATH="C:\Users\tlm\dev\imcomp\bin\Release"

set MAGICK_HOME="C:\Users\tlm\deps\win_x64\imagemagick\ImageMagick-6.9.9-35\VisualMagick"
set MAGICK_CONFIGURE_PATH="C:\Users\tlm\deps\win_x64\imagemagick\ImageMagick-6.9.9-35\VisualMagick"
set MAGICK_DEBUG=All
set MAGICK_DEBUG=None
set MAGICK_CODER_MODULE_PATH="C:\Users\tlm\deps\win_x64\imagemagick\ImageMagick-6.9.9-35\VisualMagick\coders"
set MAGICK_CODER_FILTER_PATH="C:\Users\tlm\deps\win_x64\imagemagick\ImageMagick-6.9.9-35\VisualMagick\filters"

"C:\Users\tlm\dev\imcomp\doc\dist\copy_imagemagick_dll.bat"

"C:\Users\tlm\dev\imcomp\bin\Release\imcomp_server.exe" 0.0.0.0 9973 4 "C:\Users\tlm\dev\imcomp\asset\imcomp" "C:\Users\tlm\tmp\imcomp"
"C:\Users\tlm\deps\imagemagick\ImageMagick-6.9.9-35\ImageMagick-6.9.9-35\VisualMagick\bin\convert.exe" "G:\dataset\traherne\copy1\Oxford_Balliol_College_480a13_1B01r.JPG" -resize 20% "G:\dataset\traherne\small.jpg"
```

## Code Updates
 * Update src/http_server/http_request.cc
```
http_request: ((size_t) payload_.tellg()) == content_length
```

 * conflicting snprintf macro definition
```
// Error: C:\Program Files (x86)\Windows Kits\10\Include\10.0.16299.0\ucrt\stdio.h(1933): fatal error C1189: #error:  Macro definition of snprintf conflicts with Standard Library function declaration [C:\Users\tlm\dev\imcomp\build\win_x64\src\i mreg_sift\imreg_sift.vcxproj]
// Update vl/generic.h
// added by Abhishek Dutta (20 Feb. 2018) to compile in Windows Visual Studio 2017
#if defined(VL_COMPILER_MSC) & ! defined(__DOXYGEN__)
#  undef snprintf
#endif

results in this error:
C:\Users\tlm\deps\lib\lib\vl.dll : fatal error LNK1107: invalid or corrupt file: cannot read at 0x2E8 [C:\Users\tlm\dev\imcomp\build\win_x64\src\imcomp_server.vcxproj]
```

 * Update src/imreg_sift/imreg_sift.cc
```
/* allocate buffer */
  data  = (vl_uint8*) malloc( vl_pgm_get_npixels(&pim) * vl_pgm_get_bpp(&pim) * sizeof(vl_uint8) ) ;
  fdata = (vl_sift_pix*) malloc( vl_pgm_get_npixels(&pim) * vl_pgm_get_bpp(&pim) * sizeof(vl_sift_pix) ) ;
```

 * boost filesystem multiply defined symbols found
```
 Link:
 C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Tools\MSVC\14.12.25827\bin\HostX86\x64\link.exe /ERRORREPORT:QUEUE 
 /OUT:"C:\Users\tlm\dev\imcomp\bin\Release\imcomp_server.exe" /INCREMENTAL:NO /NOLOGO 
 /LIBPATH:"C:/Users/tlm/deps/boost_1_65_1/lib64-msvc-14.1" 
 /LIBPATH:"C:/Users/tlm/deps/boost_1_65_1/lib64-msvc-14.1/Release" 
 http_server\Release\http_server.lib imcomp\Release\imcomp_request_handler.lib 
 "C:\Program Files\ImageMagick-6.9.9-Q8\lib\CORE_RL_Magick++_.lib" 
 "C:\Users\tlm\deps\boost_1_65_1\lib64-msvc-14.1\boost_filesystem-vc141-mt-1_65_1.lib" 
 "C:\Users\tlm\deps\boost_1_65_1\lib64-msvc-14.1\boost_system-vc141-mt-1_65_1.lib" 
 "C:\Users\tlm\deps\boost_1_65_1\lib64-msvc-14.1\boost_thread-vc141-mt-1_65_1.lib" 
 "C:\Users\tlm\deps\boost_1_65_1\lib64-msvc-14.1\boost_atomic-vc141-mt-1_65_1.lib" 
 "C:\Users\tlm\deps\boost_1_65_1\lib64-msvc-14.1\boost_chrono-vc141-mt-1_65_1.lib" 
 "C:\Users\tlm\deps\boost_1_65_1\lib64-msvc-14.1\boost_date_time-vc141-mt-1_65_1.lib" 
 util\Release\util.lib imreg_sift\Release\imreg_sift.lib 
 "C:\Program Files\ImageMagick-6.9.9-Q8\lib\CORE_RL_Magick++_.lib" 
 C:\Users\tlm\deps\lib\lib\vl.lib 
 "C:\Users\tlm\deps\boost_1_65_1\lib64-msvc-14.1\boost_filesystem-vc141-mt-1_65_1.lib" 
 "C:\Users\tlm\deps\boost_1_65_1\lib64-msvc-14.1\boost_system-vc141-mt-1_65_1.lib" 
 "C:\Users\tlm\deps\boost_1_65_1\lib64-msvc-14.1\boost_thread-vc141-mt-1_65_1.lib" 
 "C:\Users\tlm\deps\boost_1_65_1\lib64-msvc-14.1\boost_atomic-vc141-mt-1_65_1.lib" 
 "C:\Users\tlm\deps\boost_1_65_1\lib64-msvc-14.1\boost_chrono-vc141-mt-1_65_1.lib" 
 "C:\Users\tlm\deps\boost_1_65_1\lib64-msvc-14.1\boost_date_time-vc141-mt-1_65_1.lib"    
 http_server\Release\http_request.lib http_server\Release\http_response.lib kernel32.lib user32.lib gdi32.lib winspool.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib 
 /MANIFEST /MANIFESTUAC:"level='asInvoker'    uiAccess='false'" /manifest:embed /PDB:"C:/Users/tlm/dev/imcomp/bin/Release/imcomp_server.pdb" /SUBSYSTEM:CONSOLE /TLBID:1 /DYNAMICBASE /NXCOMPAT 
 /IMPLIB:"C:/Users/tlm/dev/imcomp/bin/Release/imcomp_server.lib" /MACHINE:X64  /machine:x   64 imcomp_server.dir\Release\imcomp_server.obj
 
 C:\Users\tlm\dev\imcomp\bin\Release\imcomp_server.exe : fatal error LNK1169: one or more multiply defined symbols found [C:\Users\tlm\dev\imcomp\build\win_x64\src\imcomp_server.vcxproj]
 
 Solution:
 	set( Boost_USE_MULTITHREADED ON )
  set( Boost_USE_DEBUG_RUNTIME OFF )
  set( Boost_USE_STATIC_RUNTIME OFF )
  set( Boost_USE_STATIC_LIBS OFF )
  set( BOOST_ALL_DYN_LINK ON)
  set( Boost_DEBUG OFF )
  add_definitions(-DBOOST_ALL_NO_LIB)
 ```

 
## Packaging Application
 * https://docs.microsoft.com/en-us/windows/uwp/porting/desktop-to-uwp-root
 * https://docs.microsoft.com/en-gb/cpp/windows/windows-desktop-applications-cpp
 * https://docs.microsoft.com/en-gb/cpp/ide/understanding-the-dependencies-of-a-visual-cpp-application
 * ImageMagick
   - https://www.imagemagick.org/discourse-server/viewtopic.php?t=26856
   - http://www.imagemagick.org/script/resources.php