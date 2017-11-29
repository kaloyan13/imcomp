cmake_minimum_required( VERSION 2.8 )
project( relja_register )

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR})

#set( BUILD_SHARED_LIBS "OFF" )

set(BUILD_SHARED_LIBS "ON" )
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
	set(CMAKE_CXX_FLAGS "--std=c++11 -Wall -DEIGEN_NO_DEBUG -msse2 -mpopcnt -O3 -funroll-loops -march=native -s -DNDEBUG")
else()
	ADD_DEFINITIONS(-D_WIN32_WINDOWS)
endif()

#set(CMAKE_CXX_FLAGS "--std=c++11 -g3 -Wall -DEIGEN_NO_DEBUG -msse2 -mpopcnt -O3 -funroll-loops -march=native -s -DNDEBUG")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g3")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -funroll-loops -march=native -s -DNDEBUG")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories( det_ransac )
include_directories( features )
include_directories( external_lib/eigen )
include_directories( util )
include_directories( external_lib/KMCode_relja/exec/detect_points )
include_directories( external_lib/KMCode_relja/exec/compute_descriptors )

add_subdirectory( det_ransac )
add_subdirectory( features )
add_subdirectory( util )
add_subdirectory( external_lib )

add_library( register_images STATIC register_images.cpp )
target_link_libraries( register_images feat_standard ellipse det_ransac image_util ${ImageMagick_LIBRARIES} ${Boost_LIBRARIES} )

## extra tools for internal use
#add_executable( putative_matches putative_matches.cc )
#target_link_libraries( putative_matches feat_standard ellipse det_ransac image_util ${Boost_LIBRARIES} )

#add_executable( ransac_inliers ransac_inliers.cc )
#target_link_libraries( ransac_inliers feat_standard ellipse det_ransac image_util ${Boost_LIBRARIES} )
