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
set (IMCOMP_SERVER_VERSION_PATCH "3")
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
IF(WIN32)
  SET(CMAKE_FIND_LIBRARY_SUFFIXES .lib .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
ELSE(WIN32)
  SET(CMAKE_FIND_LIBRARY_SUFFIXES .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
ENDIF(WIN32)

IF(CMAKE_BUILD_TYPE MATCHES Debug)
  message("*** Debug Mode: test code will be compiled and built. ***")
ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
	set(CMAKE_CXX_FLAGS "-fpermissive")
endif()
  
if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
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

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  #set(ImageMagick_INCLUDE_DIRS "C:\\Users\\tlm\\deps\\win_x64\\imagemagick\\ImageMagick-6.9.8-10\\ImageMagick;C:\\Users\\tlm\\deps\\win_x64\\imagemagick\\ImageMagick-6.9.8-10\\ImageMagick\\Magick++\\lib;C:\\Users\\tlm\\deps\\win_x64\\imagemagick\\ImageMagick-6.9.8-10\\ImageMagick\\Magick++\\lib\\Magick++")
  #set(ImageMagick_LIBRARY_DIRS "C:\\Users\\tlm\\deps\\win_x64\\imagemagick\\ImageMagick-6.9.8-10\\VisualMagick\\lib")
  #set(ImageMagick_LIBRARIES "C:\\Users\\tlm\\deps\\win_x64\\imagemagick\\ImageMagick-6.9.8-10\\VisualMagick\\lib\\CORE_DB_Magick++_.lib")
  #include_directories( ${ImageMagick_INCLUDE_DIRS} )
  #link_directories( ${ImageMagick_LIBRARY_DIRS} )
  
  find_package( ImageMagick COMPONENTS Magick++ REQUIRED )
  include_directories( ${ImageMagick_INCLUDE_DIRS} )
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
  SET(CPACK_PACKAGE_NAME "Traherne")
  SET(CPACK_PACKAGE_VENDOR "Abhishek Dutta <adutta@robots.ox.ac.uk>")
  SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Traherne Digital Collator")
  SET(CPACK_PACKAGE_VERSION "${IMCOMP_SERVER_VERSION_MAJOR}.${IMCOMP_SERVER_VERSION_MINOR}.${IMCOMP_SERVER_VERSION_PATCH}")
  SET(CPACK_PACKAGE_VERSION_MAJOR ${IMCOMP_SERVER_VERSION_MAJOR})
  SET(CPACK_PACKAGE_VERSION_MINOR ${IMCOMP_SERVER_VERSION_MINOR})
  SET(CPACK_PACKAGE_VERSION_PATCH ${IMCOMP_SERVER_VERSION_PATCH})
  SET(CPACK_PACKAGE_INSTALL_DIRECTORY "Traherne Digital Collator")
  SET(CPACK_PACKAGE_EXECUTABLES "Traherne" "Traherne Digital Collator")
  SET(CPACK_SYSTEM_NAME "Win64")
  SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
  SET(CPACK_CREATE_DESKTOP_LINKS "bin\\\\Traherne.exe")
  #SET(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/asset/images/traherne/traherne_48x48.ico")

  SET(CPACK_NSIS_CONTACT "Abhishek Dutta <adutta@robots.ox.ac.uk>")
  #SET(CPACK_NSIS_CREATE_ICONS_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/asset/images/traherne/traherne_48x48.ico")
  SET(CPACK_NSIS_DISPLAY_NAME "Traherne Digital Collator")
  SET(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL "ON")
  SET(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\Traherne.exe")
  SET(CPACK_NSIS_HELP_LINK "http://www.robots.ox.ac.uk/~vgg/software/imcomp/traherne/")
  SET(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/asset/imcomp/traherne/images/traherne/traherne_48x48.ico")
  SET(CPACK_NSIS_PACKAGE_NAME "Traherne Digital Collator")
  SET(CPACK_NSIS_URL_INFO_ABOUT "http://www.robots.ox.ac.uk/~vgg/software/imcomp/traherne/")
  
  # @todo avoid hard coded paths
  INSTALL(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/asset" DESTINATION "bin")
  INSTALL(DIRECTORY "C:\\\\Users\\\\tlm\\\\deps\\\\win_x64\\\\cpack_libs\\\\" DESTINATION "bin")
  INSTALL(FILES "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE" DESTINATION ".")
  INSTALL(FILES "${CMAKE_CURRENT_SOURCE_DIR}/README.md" DESTINATION ".")
  INSTALL(FILES "${CMAKE_CURRENT_SOURCE_DIR}/CHANGELOG" DESTINATION ".")
  INSTALL(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/third_party_licenses" DESTINATION ".")
  
  INCLUDE(CPack)
endif()
