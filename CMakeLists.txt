##
## imcomp_server
##
## Author: Abhishek Dutta <adutta@robots.ox.ac.uk>
## 27 Nov. 2017
##

cmake_minimum_required( VERSION 3.10 )
project( imcomp_server )

set( PROJECT_INCLUDE_DIR "include" )
set( PROJECT_SOURCE_DIR "src" )
set( PROJECT_BINARY_DIR "bin" )
set( PROJECT_TEST_DIR "test" )
set( PROJECT_STATIC_DIR "static" )

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

## export version number in configure header file
set (IMCOMP_SERVER_NAME "imcomp_server")
set (IMCOMP_SERVER_VERSION_MAJOR "2")
set (IMCOMP_SERVER_VERSION_MINOR "0")
#set (IMCOMP_SERVER_VERSION_PATCH "x") # traherne or imcomp user interface version to be set by user
set (IMCOMP_SERVER_URL "http://www.robots.ox.ac.uk/~vgg/software/imcomp/")
set (IMCOMP_SERVER_AUTHOR_NAME "Abhishek Dutta")
set (IMCOMP_SERVER_AUTHOR_EMAIL "adutta@robots.ox.ac.uk")
set (IMCOMP_SERVER_FIRST_RELEASE_DATE "Nov. 2017")
set (IMCOMP_SERVER_CURRENT_RELEASE_DATE "02 Mar. 2018")
configure_file (
  "${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_SOURCE_DIR}/imcomp_server_config.h.in" 
  "${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_INCLUDE_DIR}/imcomp_server_config.h"
  )

set(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set( BUILD_SHARED_LIBS "OFF" )

IF(CMAKE_BUILD_TYPE MATCHES Debug)
  message("*** Debug Mode: test code will be compiled and built. ***")
ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
	set(CMAKE_CXX_FLAGS "-fpermissive")
endif()

##
## Define variables for Windows build
## @todo avoid hard coded paths
##
if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  set(CMAKE_CXX_FLAGS_RELEASE "-Ox")
  if(CMAKE_GENERATOR_PLATFORM MATCHES "x64")
    SET (VLFEAT_LIB "C:\\Users\\tlm\\build\\imcomp\\deps\\x64\\vlfeat\\vlfeat-0.9.21\\bin\\win64\\vl.lib")
    SET (VLFEAT_INCLUDE_DIR "C:\\Users\\tlm\\build\\imcomp\\deps\\x64\\vlfeat\\vlfeat-0.9.21")
    SET (IMCOMP_DEPS_BASEDIR "C:\\Users\\tlm\\build\\imcomp\\deps\\x64")
    SET (BOOST_ROOT "C:\\Users\\tlm\\build\\imcomp\\deps\\x64\\boost_1_65_1")
    SET (Eigen3_DIR "C:\\Users\\tlm\\build\\imcomp\\deps\\x64\\eigen-eigen-5a0156e40feb\\build")
  else()
    SET (VLFEAT_LIB "C:\\Users\\tlm\\build\\imcomp\\deps\\x86\\vlfeat\\vlfeat-0.9.21\\bin\\win32\\vl.lib")
    SET (VLFEAT_INCLUDE_DIR "C:\\Users\\tlm\\build\\imcomp\\deps\\x86\\vlfeat\\vlfeat-0.9.21")
    SET (IMCOMP_DEPS_BASEDIR "C:\\Users\\tlm\\build\\imcomp\\deps\\x86")
    SET (BOOST_ROOT "C:\\Users\\tlm\\build\\imcomp\\deps\\x86\\boost_1_65_1")
    SET (Eigen3_DIR "C:\\Users\\tlm\\build\\imcomp\\deps\\x86\\eigen-eigen-5a0156e40feb\\build")
  endif()
  STRING(REPLACE "\\" "\\\\" IMCOMP_DEPS_BASEDIR_2SLASH ${IMCOMP_DEPS_BASEDIR})
  STRING(REPLACE "\\" "\\\\" IMCOMP_DEPS_BASEDIR_4SLASH ${IMCOMP_DEPS_BASEDIR_2SLASH})
  #message("IMCOMP_DEPS_BASEDIR: " ${IMCOMP_DEPS_BASEDIR})
  #message("IMCOMP_DEPS_BASEDIR_2SLASH: " ${IMCOMP_DEPS_BASEDIR_2SLASH})
  #message("IMCOMP_DEPS_BASEDIR_4SLASH: " ${IMCOMP_DEPS_BASEDIR_4SLASH})
  ADD_DEFINITIONS(-D_WIN32_WINDOWS)
endif()

## Boost
find_package(Boost 1.64.0 COMPONENTS filesystem system thread atomic REQUIRED)
if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	set( Boost_USE_MULTITHREADED ON )
  set( Boost_USE_DEBUG_RUNTIME OFF )
  set( Boost_USE_STATIC_RUNTIME OFF )
  set( Boost_USE_STATIC_LIBS OFF )
  set( BOOST_ALL_DYN_LINK ON)
  set( Boost_DEBUG OFF )
  add_definitions(-DBOOST_ALL_NO_LIB)
  add_definitions(${Boost_LIB_DIAGNOSTIC_DEFINITIONS})
endif()
link_directories( ${Boost_LIBRARY_DIRS} )
include_directories( ${Boost_INCLUDE_DIRS} )

## ImageMagick
ADD_DEFINITIONS(-DMAGICKCORE_HDRI_ENABLE=0)
ADD_DEFINITIONS(-DMAGICKCORE_QUANTUM_DEPTH=8)
if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  set(ImageMagick_INCLUDE_DIRS "\
${IMCOMP_DEPS_BASEDIR_2SLASH}\\ImageMagick-6.9.9-36\\ImageMagick;\
${IMCOMP_DEPS_BASEDIR_2SLASH}\\ImageMagick-6.9.9-36\\ImageMagick\\Magick++\\lib;\
")
  include_directories( ${ImageMagick_INCLUDE_DIRS} )
  SET(ImageMagick_LIBRARIES "CORE_RL_Magick++_;CORE_RL_magick_;CORE_RL_wand_")
  link_directories("${IMCOMP_DEPS_BASEDIR_2SLASH}\\ImageMagick-6.9.9-36\\VisualMagick\\lib")
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  find_package( ImageMagick COMPONENTS Magick++ REQUIRED )
  # @todo: on alienware, find_package() points to "/usr/lib/x86_64-linux-gnu/libMagick++-6.Q16.so"
  # and does not detect the library available at "/home/tlm/deps/imcomp/lib/lib/libMagick++-6.Q8.so"
  set(ImageMagick_LIBRARIES "/home/tlm/deps/imcomp/lib/lib/libMagick++-6.Q8.so")
  include_directories( ${ImageMagick_INCLUDE_DIRS} )
endif()

# Eigen
# requires EIGEN_DIR to be set to the location where Eigen3 is built using cmake
find_package (Eigen3 3.3 REQUIRED NO_MODULE)

# VLFEAT
# VLFEAT_LIB must be define during cmake
# e.g. cmake -DVLFEAT_LIB=/data/mybin/vlfeat/vlfeat-0.9.20/bin/glnxa64/libvl.so -DVLFEAT_INCLUDE_DIR=/data/mybin/vlfeat/vlfeat-0.9.20/vl
# @todo: get rid of absolute path
include_directories( ${VLFEAT_INCLUDE_DIR})

message("Boost_LIBRARIES: " ${Boost_LIBRARIES})
message("Boost_INCLUDE_DIRS: " ${Boost_INCLUDE_DIRS})
message("ImageMagick_LIBRARIES: " ${ImageMagick_LIBRARIES} )
message("ImageMagick_INCLUDE_DIRS: " ${ImageMagick_INCLUDE_DIRS} )
message("VLFEAT_LIB: " ${VLFEAT_LIB} )
message("VLFEAT_INCLUDE_DIR: " ${VLFEAT_INCLUDE_DIR} )

include_directories("${PROJECT_INCLUDE_DIR}")
add_subdirectory("${PROJECT_SOURCE_DIR}" )
add_subdirectory("${PROJECT_TEST_DIR}" )

##
## Packaging installer using cpack
##
## See
## - https://github.com/mgalloy/cpack-cheatsheets/blob/master/cpack-variables.rst
if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  INCLUDE(InstallRequiredSystemLibraries)
  SET(CPACK_GENERATOR "NSIS")
  SET(CPACK_PACKAGE_NAME "${RELEASE_PACKAGE_NAME}")
  SET(CPACK_PACKAGE_VENDOR "Abhishek Dutta <adutta@robots.ox.ac.uk>")
  SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${RELEASE_PACKAGE_DESCRIPTION}")
  SET(CPACK_PACKAGE_VERSION "${IMCOMP_SERVER_VERSION_MAJOR}.${IMCOMP_SERVER_VERSION_MINOR}.${IMCOMP_SERVER_VERSION_PATCH}")
  SET(CPACK_PACKAGE_VERSION_MAJOR ${IMCOMP_SERVER_VERSION_MAJOR})
  SET(CPACK_PACKAGE_VERSION_MINOR ${IMCOMP_SERVER_VERSION_MINOR})
  SET(CPACK_PACKAGE_VERSION_PATCH ${IMCOMP_SERVER_VERSION_PATCH})
  SET(CPACK_PACKAGE_INSTALL_DIRECTORY "${RELEASE_PACKAGE_DESCRIPTION}")
  
  if(CMAKE_GENERATOR_PLATFORM MATCHES "x64")
    SET(CPACK_PACKAGE_EXECUTABLES "${RELEASE_PACKAGE_NAME}" "${RELEASE_PACKAGE_DESCRIPTION}")
    SET(CPACK_NSIS_DISPLAY_NAME "${RELEASE_PACKAGE_DESCRIPTION}")
    SET(CPACK_SYSTEM_NAME "Win64")
    
    INSTALL(DIRECTORY "${IMCOMP_DEPS_BASEDIR_4SLASH}\\\\cpack_win_deps_dll\\\\" DESTINATION "bin")
  else()
    SET(CPACK_PACKAGE_EXECUTABLES "${RELEASE_PACKAGE_NAME}" "${RELEASE_PACKAGE_DESCRIPTION} (32 bit)")
    SET(CPACK_NSIS_DISPLAY_NAME "${RELEASE_PACKAGE_DESCRIPTION} (32 bit)")
    SET(CPACK_SYSTEM_NAME "Win32")
    
    INSTALL(DIRECTORY "${IMCOMP_DEPS_BASEDIR_4SLASH}\\\\cpack_win_deps_dll\\\\" DESTINATION "bin")
  endif()
  SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
  SET(CPACK_CREATE_DESKTOP_LINKS "bin\\\\${RELEASE_PACKAGE_NAME}.exe")
  #SET(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/asset/images/traherne/traherne_48x48.ico")

  SET(CPACK_NSIS_CONTACT "Abhishek Dutta <adutta@robots.ox.ac.uk>")
  #SET(CPACK_NSIS_CREATE_ICONS_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/asset/images/traherne/traherne_48x48.ico")
  
  SET(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL "ON")
  SET(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\${RELEASE_PACKAGE_NAME}.exe")
  SET(CPACK_NSIS_HELP_LINK "http://www.robots.ox.ac.uk/~vgg/software/imcomp/traherne/")
  SET(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/asset/imcomp/traherne/images/traherne/traherne_48x48.ico")
  SET(CPACK_NSIS_PACKAGE_NAME "${RELEASE_PACKAGE_DESCRIPTION}")
  SET(CPACK_NSIS_URL_INFO_ABOUT "http://www.robots.ox.ac.uk/~vgg/software/imcomp/traherne/")
  
  # @todo avoid hard coded paths
  INSTALL(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/asset" DESTINATION "bin")
  INSTALL(FILES "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt" DESTINATION ".")
  INSTALL(FILES "${CMAKE_CURRENT_SOURCE_DIR}/README.md" DESTINATION ".")
  INSTALL(FILES "${CMAKE_CURRENT_SOURCE_DIR}/CHANGELOG" DESTINATION ".")
  INSTALL(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/third_party_licenses" DESTINATION ".")
  
  INCLUDE(CPack)
endif()
