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
SET IMCOMP_SERVER_VERSION_PATCH="4" REM Traherne user interface version

SET BUILD_BASE_FOLDER="C:\Users\tlm\build\imcomp\release"
SET CMAKE_EXEC="C:\Program Files\CMake\bin\cmake.exe"
SET CMAKE_GENERATOR_NAME="Visual Studio 15 2017"
SET IMCOMP_SOURCE_PATH="C:\Users\tlm\build\imcomp\src\imcomp"

::
:: Build release for Windows 32 bit
::
SET BUILD_BASE_FOLDER_WIN32=%BUILD_BASE_FOLDER%\Win32
IF NOT EXIST %BUILD_BASE_FOLDER_WIN32% (
  mkdir %BUILD_BASE_FOLDER_WIN32%
  cd /d %BUILD_BASE_FOLDER_WIN32%
  %CMAKE_EXEC% ^
   -G %CMAKE_GENERATOR_NAME% ^
   -DCMAKE_BUILD_TYPE=Release ^
   -DRELEASE_PACKAGE_NAME=%RELEASE_PACKAGE_NAME% ^
   -DIMCOMP_SERVER_VERSION_PATCH=%IMCOMP_SERVER_VERSION_PATCH% ^
   -DRELEASE_PACKAGE_DESCRIPTION=%RELEASE_PACKAGE_DESCRIPTION% ^
   %IMCOMP_SOURCE_PATH%
  msbuild PACKAGE.vcxproj /maxcpucount:8 -p:PreferredToolArchitecture=x86 /nologo /p:configuration=Release
)

::
:: Build release for Windows 64 bit
::
SET BUILD_BASE_FOLDER_WIN64=%BUILD_BASE_FOLDER%\Win64
IF NOT EXIST %BUILD_BASE_FOLDER_WIN64% (
  mkdir %BUILD_BASE_FOLDER_WIN64%
  cd /d %BUILD_BASE_FOLDER_WIN64%
  %CMAKE_EXEC% ^
   -G %CMAKE_GENERATOR_NAME% ^
   -DCMAKE_BUILD_TYPE=Release ^
   -DCMAKE_GENERATOR_PLATFORM=x64 ^
   -DRELEASE_PACKAGE_NAME=%RELEASE_PACKAGE_NAME% ^
   -DIMCOMP_SERVER_VERSION_PATCH=%IMCOMP_SERVER_VERSION_PATCH% ^
   -DRELEASE_PACKAGE_DESCRIPTION=%RELEASE_PACKAGE_DESCRIPTION% ^
   %IMCOMP_SOURCE_PATH%
  msbuild PACKAGE.vcxproj /maxcpucount:8 -p:PreferredToolArchitecture=x64 /nologo /p:configuration=Release
)

cd %CURRENT_PATH%