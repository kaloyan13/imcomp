add_library( imcomp_request_handler imcomp_request_handler.cc imcomp_asset.cc)
target_link_libraries( imcomp_request_handler http_request http_response util imreg_sift ${ImageMagick_LIBRARIES} ${Boost_LIBRARIES} )

