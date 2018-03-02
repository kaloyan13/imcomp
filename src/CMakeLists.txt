add_subdirectory( http_server )
add_subdirectory( imcomp )
add_subdirectory( util )
add_subdirectory( imreg_sift )

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_executable( imcomp_server imcomp_server.cc )
  target_link_libraries( imcomp_server http_server imcomp_request_handler ${ImageMagick_LIBRARIES} ${Boost_LIBRARIES})
else()
  if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    if(RELEASE_PACKAGE_NAME MATCHES "Traherne")
      # Create installer in Windows
      add_executable( Traherne imcomp_server.cc ${WINDOWS_RESOURCE_FILENAME})
      #add_executable( Traherne imcomp_server.cc)
      #add_executable( Traherne WIN32 imcomp_server.cc ) # to make this a native GUI application
      target_link_libraries( Traherne http_server imcomp_request_handler ${ImageMagick_LIBRARIES} ${Boost_LIBRARIES})
      install(TARGETS Traherne DESTINATION bin)
    else()
      add_executable( Imcomp imcomp_server.cc)
      #add_executable( Traherne WIN32 imcomp_server.cc ) # to make this a native GUI application
      target_link_libraries( Imcomp http_server imcomp_request_handler ${ImageMagick_LIBRARIES} ${Boost_LIBRARIES})
      install(TARGETS Imcomp DESTINATION bin)
    endif()
  endif()
endif()

