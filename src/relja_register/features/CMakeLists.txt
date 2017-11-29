add_library( feat_getter STATIC feat_getter.cpp )
target_link_libraries( feat_getter ellipse ${ImageMagick_LIBRARIES} )

add_library( feat_standard STATIC feat_standard.cpp )
target_link_libraries( feat_standard
    compute_descriptors
    detect_points
    feat_getter
    image_util
    ${Boost_LIBRARIES} )
