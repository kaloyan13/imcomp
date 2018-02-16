##
## http_server
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
set (IMCOMP_SERVER_NAME "imcomp server")
set (IMCOMP_SERVER_VERSION_MAJOR "2")
set (IMCOMP_SERVER_VERSION_MINOR "0")
set (IMCOMP_SERVER_VERSION_PATCH "2")
set (IMCOMP_SERVER_AUTHOR_NAME "Abhishek Dutta")
set (IMCOMP_SERVER_AUTHOR_EMAIL "adutta@robots.ox.ac.uk")
set (IMCOMP_SERVER_FIRST_RELEASE "Nov. 2017")
configure_file (
  "${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_SOURCE_DIR}/imcomp_server_config.h.in" 
  "${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_INCLUDE_DIR}/imcomp_server_config.h"
  )

set(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/bin")
set( BUILD_SHARED_LIBS "OFF" )

IF(CMAKE_BUILD_TYPE MATCHES Debug)
  message("*** Debug Mode: test code will be compiled and built. ***")
ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)

## Boost
if (NOT DEFINED     BOOST_ROOT        AND
    NOT DEFINED ENV{BOOST_ROOT}       AND
    NOT DEFINED     BOOST_INCLUDEDIR  AND
    NOT DEFINED ENV{BOOST_INCLUDEDIR} AND
    NOT DEFINED     BOOST_LIBRARYDIR  AND
    NOT DEFINED ENV{BOOST_LIBRARYDIR})
    if (APPLE)
        set(BOOST_ROOT "@todo")
    elseif (WIN32)
        set(BOOST_ROOT "C:/Users/tlm/deps/boost_1_65_1")
		set(BOOST_INCLUDEDIR "C:/Users/tlm/deps/boost_1_65_1")
        set(BOOST_LIBRARYDIR "C:/Users/tlm/deps/boost_1_65_1/lib64-msvc-14.1")
    endif()
endif()

find_package(Boost 1.64.0 COMPONENTS filesystem system thread atomic REQUIRED)
link_directories( ${Boost_LIBRARY_DIRS} )
include_directories( ${Boost_INCLUDE_DIRS} )

## Magick++
#if (NOT DEFINED     ImageMagick_LIBRARIES        AND
#    NOT DEFINED ENV{ImageMagick_LIBRARIES}       AND
#    NOT DEFINED     ImageMagick_INCLUDE_DIRS  AND
#    NOT DEFINED ENV{ImageMagick_INCLUDE_DIRS})
#    if (APPLE)
#        set(ImageMagick_LIBRARIES "@todo")
#		set(ImageMagick_INCLUDE_DIRS "@todo")
#    elseif (WIN32)
#		set(ImageMagick_LIBRARIES "C:/Program Files/ImageMagick-6.9.9-Q8/lib;C:/Program Files/ImageMagick-6.9.9-Q8")
#		set(ImageMagick_INCLUDE_DIRS "C:/Program Files/ImageMagick-6.9.9-Q8/include")
#    endif()
#endif()

ADD_DEFINITIONS(-DMAGICKCORE_HDRI_ENABLE=0)
ADD_DEFINITIONS(-DMAGICKCORE_QUANTUM_DEPTH=8)
find_package( ImageMagick COMPONENTS Magick++ REQUIRED )
# @todo: on alienware, find_package() points to "/usr/lib/x86_64-linux-gnu/libMagick++-6.Q16.so"
# and does not detect the library available at "/home/tlm/deps/imcomp/lib/lib/libMagick++-6.Q8.so"
set(ImageMagick_LIBRARIES "/home/tlm/deps/imcomp/lib/lib/libMagick++-6.Q8.so")
include_directories( ${ImageMagick_INCLUDE_DIRS} )

# Eigen
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
