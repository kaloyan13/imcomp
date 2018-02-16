include_directories( imreg_sift )

add_subdirectory( http_server )
add_subdirectory( imcomp )
add_subdirectory( util )
add_subdirectory( imreg_sift )

add_executable( imcomp_server imcomp_server.cc )
target_link_libraries( imcomp_server http_server imcomp_request_handler ${ImageMagick_LIBRARIES} ${Boost_LIBRARIES})

