set(BUILD_SHARED_LIBS "ON" )
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
	set(CMAKE_CXX_FLAGS "--std=c++11 -fpermissive")
else()
	ADD_DEFINITIONS(-D_WIN32_WINDOWS)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# VLFEAT
set(VLFEAT_LIBRARIES "/data/mybin/vlfeat/vlfeat-0.9.20/bin/glnxa64/libvl.so")

add_subdirectory( external_lib )

add_library( vl_register_images STATIC vl_register_images.cc )
target_link_libraries( vl_register_images ${ImageMagick_LIBRARIES} ${Boost_LIBRARIES} ${VLFEAT_LIBRARIES})

