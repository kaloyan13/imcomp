include_directories( relja_register/det_ransac )
include_directories( relja_register/features )
include_directories( relja_register/external_lib/eigen )
include_directories( relja_register/util )
include_directories( relja_register/external_lib/KMCode_relja/exec/detect_points )
include_directories( relja_register/external_lib/KMCode_relja/exec/compute_descriptors )

add_subdirectory( http_server )
add_subdirectory( imcomp )
add_subdirectory( relja_register )
add_subdirectory( util )

add_executable( imcomp_server imcomp_server.cc )
target_link_libraries( imcomp_server http_server imcomp_request_handler ${ImageMagick_LIBRARIES} ${Boost_LIBRARIES})

