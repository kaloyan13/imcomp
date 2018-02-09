set(BUILD_SHARED_LIBS "ON" )
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
	set(CMAKE_CXX_FLAGS "--std=c++11 -fpermissive")
else()
	ADD_DEFINITIONS(-D_WIN32_WINDOWS)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library( vl_register_images STATIC vl_register_images.cc )
target_link_libraries( vl_register_images ${ImageMagick_LIBRARIES} ${Boost_LIBRARIES} ${VLFEAT_LIB} Eigen3::Eigen)

add_executable( test_vl_register_images test_vl_register_images.cc )
target_link_libraries( test_vl_register_images vl_register_images ${ImageMagick_LIBRARIES} ${Boost_LIBRARIES})

