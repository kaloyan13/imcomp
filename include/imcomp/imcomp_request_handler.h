/** @file   imcomp_request_handler.h
 *  @brief  A singleton class which handles http requests related to imcomp
 *
 *
 *  @author Abhishek Dutta (adutta@robots.ox.ac.uk)
 *  @date   29 Nov 2017
 */

#ifndef _IMCOMP_REQUEST_HANDLER_H_
#define _IMCOMP_REQUEST_HANDLER_H_

#include <iostream>

#include <boost/filesystem.hpp>

#include "http_server/http_request.h"
#include "http_server/http_response.h"
#include "imcomp/imcomp_asset.h"

#include "util/strutil.h"

using namespace std;

// uses C++ singleton design pattern
class imcomp_request_handler {
  boost::filesystem::path upload_dir_;
  boost::filesystem::path result_dir_;

  imcomp_request_handler() { };
  imcomp_request_handler(const imcomp_request_handler& sh) { };
  imcomp_request_handler* operator=(const imcomp_request_handler &) {
    return 0;
  }

  static imcomp_request_handler* imcomp_request_handler_;

  public:
  static imcomp_request_handler* instance();

  void init(const boost::filesystem::path upload_dir, const boost::filesystem::path result_dir);
  void handle_http_request(const http_request& request, http_response& response);
};
#endif
