add_library( same_random STATIC same_random.cpp )
target_link_libraries( same_random ${Boost_LIBRARIES} )

add_library( image_util STATIC image_util.cpp )
target_link_libraries( image_util ${ImageMagick_LIBRARIES} ${Boost_LIBRARIES} )
