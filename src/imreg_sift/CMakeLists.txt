set(BUILD_SHARED_LIBS "ON" )
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
	set(CMAKE_CXX_FLAGS "--std=c++11 -fpermissive")
else()
	ADD_DEFINITIONS(-D_WIN32_WINDOWS)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library( imreg_sift STATIC imreg_sift.cc )
target_link_libraries( imreg_sift ${ImageMagick_LIBRARIES} ${Boost_LIBRARIES} ${VLFEAT_LIB} Eigen3::Eigen)

IF(CMAKE_BUILD_TYPE MATCHES Debug)
  message("debug mode")
  add_executable( test_imreg_sift test_imreg_sift.cc )
  target_link_libraries( test_imreg_sift imreg_sift ${ImageMagick_LIBRARIES} ${Boost_LIBRARIES})
ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)

