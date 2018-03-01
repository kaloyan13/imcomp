::
:: Script to build Windows releases
:: Author: Abhishek Dutta <adutta@robots.ox.ac.uk>
:: March 01, 2018
::

@ECHO OFF
cls

SET CURRENT_PATH=%cd%

SET RELEASE_PACKAGE_NAME="Traherne"
SET RELEASE_PACKAGE_DESCRIPTION="Traherne Digital Collator"
SET IMCOMP_SERVER_VERSION_PATCH="3" REM Traherne user interface version

SET BUILD_BASE_FOLDER="G:\build\imcomp\release"
SET CPACK_WIN_DEPS_DLL_BASE_FOLDER="G:\build\imcomp\deps\cpack_win_deps_dll"
SET CMAKE_EXEC="c:\Program Files\CMake\bin\cmake.exe"
SET CMAKE_GENERATOR_NAME="Visual Studio 15 2017"
SET IMCOMP_SOURCE_PATH="C:\Users\tlm\dev\imcomp"

::
:: Build release for Windows 32 bit
::
SET BUILD_BASE_FOLDER_WIN32=%BUILD_BASE_FOLDER%\Win32
mkdir %BUILD_BASE_FOLDER_WIN32%
cd /d %BUILD_BASE_FOLDER_WIN32%

:: configure
%CMAKE_EXEC% ^
 -G %CMAKE_GENERATOR_NAME% ^
 -DCMAKE_BUILD_TYPE=Release ^
 -DRELEASE_PACKAGE_NAME=%RELEASE_PACKAGE_NAME% ^
 -DIMCOMP_SERVER_VERSION_PATCH=%IMCOMP_SERVER_VERSION_PATCH% ^
 -DRELEASE_PACKAGE_DESCRIPTION=%RELEASE_PACKAGE_DESCRIPTION% ^
 -DIMCOMP_SERVER_VERSION_PATCH=%IMCOMP_SERVER_VERSION_PATCH% ^
 -DVLFEAT_LIB="G:\build\imcomp\deps\deps_source\x86\vlfeat\vlfeat-0.9.21\bin\win32\vl.lib" ^
 -DVLFEAT_INCLUDE_DIR="G:\build\imcomp\deps\deps_source\x86\vlfeat\vlfeat-0.9.21\" ^
 -DBOOST_ROOT="G:\build\imcomp\deps\deps_source\x86\boost_1_65_1" ^
 -DIMCOMP_DEPS_BASEDIR= "G:\\build\\imcomp\\deps\\x86" ^
 %IMCOMP_SOURCE_PATH%

:: build
::msbuild PACKAGE.vcxproj /maxcpucount:8 -p:PreferredToolArchitecture=x86 /nologo /p:configuration=Release
