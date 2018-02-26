add_subdirectory( http_server )
add_subdirectory( imcomp )
add_subdirectory( util )
add_subdirectory( imreg_sift )

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_executable( imcomp_server imcomp_server.cc )
  target_link_libraries( imcomp_server http_server imcomp_request_handler ${ImageMagick_LIBRARIES} ${Boost_LIBRARIES})
else()
  # Create installer in Windows
  add_executable( Traherne imcomp_server.cc )
  target_link_libraries( Traherne http_server imcomp_request_handler ${ImageMagick_LIBRARIES} ${Boost_LIBRARIES})
  install(TARGETS Traherne DESTINATION bin)
endif()

