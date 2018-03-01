:: Script to build Windows releases

SET CPACK_WIN_DEPS_DLL_BASE_FOLDER="G:\build\imcomp\deps\cpack_win_deps_dll"
SET CMAKE_EXEC="c:\Program Files\CMake\bin\cmake.exe"
SET CMAKE_GENERATOR_NAME="Visual Studio 15 2017"
SET IMCOMP_SOURCE_PATH="C:\Users\tlm\dev\imcomp"
SET CMAKE_PREFIX_PATH="C:\Users\tlm\deps\win_x86\lib\include\eigen"
SET VL_FEAT_LIB_PATH="C:\Users\tlm\deps\win_x86\lib\lib\vl.lib"
SET VL_FEAT_INCLUDE_DIR="C:\Users\tlm\deps\win_x86\lib\include"
SET BOOST_ROOT_DIR="C:\Users\tlm\deps\win_x86\boost_1_65_1"

:: First build Win32 release

"c:\Program Files\CMake\bin\cmake.exe" -G "Visual Studio 15 2017" "C:\Users\tlm\dev\imcomp" -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="C:\Users\tlm\deps\win_x86\lib\include\eigen" -DVLFEAT_LIB="C:\Users\tlm\deps\win_x86\lib\lib\vl.lib" -DVLFEAT_INCLUDE_DIR="C:\Users\tlm\deps\win_x86\lib\include" -DBOOST_ROOT="C:\Users\tlm\deps\win_x86\boost_1_65_1"