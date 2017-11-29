##
## http_server
##
## Author: Abhishek Dutta <adutta@robots.ox.ac.uk>
## 27 Nov. 2017
##

cmake_minimum_required( VERSION 3.1 )
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
set (IMCOMP_SERVER_VERSION_PATCH "1")
set (IMCOMP_SERVER_AUTHOR_NAME "Abhishek Dutta")
set (IMCOMP_SERVER_AUTHOR_EMAIL "adutta@robots.ox.ac.uk")
set (IMCOMP_SERVER_FIRST_RELEASE "Nov. 2017")
configure_file (
  "${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_SOURCE_DIR}/imcomp_server_config.h.in" 
  "${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_INCLUDE_DIR}/imcomp_server_config.h"
  )

set(EXECUTABLE_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/bin")

## needed by KMcode_relja
find_package( JPEG )
find_package( PNG )

## Boost
find_package(Boost 1.48.0 COMPONENTS filesystem system thread atomic REQUIRED)
link_directories( ${Boost_LIBRARY_DIRS} )
include_directories( ${Boost_INCLUDE_DIRS} )

## Magick++
find_package( ImageMagick COMPONENTS Magick++ REQUIRED )
include_directories( ${ImageMagick_INCLUDE_DIRS} )
ADD_DEFINITIONS(-DRR_MAGICK)
ADD_DEFINITIONS(-DMAGICKCORE_HDRI_ENABLE=0)
ADD_DEFINITIONS(-DMAGICKCORE_QUANTUM_DEPTH=16)

message("JPEG_LIBRARIES: " ${JPEG_LIBRARIES})
message("JPEG_INCLUDE_DIR: " ${JPEG_INCLUDE_DIR})
message("ZLIB_LIBRARIES: " ${ZLIB_LIBRARIES})
message("ZLIB_INCLUDE_DIRS: " ${ZLIB_INCLUDE_DIRS})
message("PNG_LIBRARIES: " ${PNG_LIBRARIES})
message("PNG_INCLUDE_DIRS: " ${PNG_INCLUDE_DIRS})

message("Boost_LIBRARIES: " ${Boost_LIBRARIES})
message("Boost_INCLUDE_DIRS: " ${Boost_INCLUDE_DIRS})
message("ImageMagick_LIBRARIES: " ${ImageMagick_LIBRARIES} )
message("ImageMagick_INCLUDE_DIRS: " ${ImageMagick_INCLUDE_DIRS} )

## pack static resources (html, css, js, etc) as compiled C++ asset
set(PACK_IMCOMP_ASSET_CMD "${CMAKE_CURRENT_SOURCE_DIR}/scripts/pack_imcomp_asset.sh")
set(ASSET_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/static/")
set(TARGET_CPP_ASSET_FILE "${CMAKE_CURRENT_SOURCE_DIR}/src/imcomp/imcomp_asset.cc")
message("ASSET_SRC_DIR: " ${ASSET_SRC_DIR})
message("TARGET_CPP_ASSET_FILE: " ${TARGET_CPP_ASSET_FILE})
message("PACK_IMCOMP_ASSET_CMD: " ${PACK_IMCOMP_ASSET_CMD})

execute_process( COMMAND ${PACK_IMCOMP_ASSET_CMD} ${ASSET_SRC_DIR} ${TARGET_CPP_ASSET_FILE}
  RESULT_VARIABLE CMD_ERR
  OUTPUT_VARIABLE CMD_OUTPUT)
MESSAGE( STATUS "Result of packing static resources : " ${CMD_OUTPUT})

include_directories("${PROJECT_INCLUDE_DIR}")
add_subdirectory("${PROJECT_SOURCE_DIR}" )
add_subdirectory("${PROJECT_TEST_DIR}" )


