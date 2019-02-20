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
#include <sstream>
#include <fstream>

#include <boost/filesystem.hpp>

// to generate uuid
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <Magick++.h>            // to transform images

#include <Eigen/Dense>

#include "http_server/http_request.h"
#include "http_server/http_response.h"

#include "util/strutil.h"
#include "imreg_sift/imreg_sift.h"

using namespace std;
using namespace Eigen;

// uses C++ singleton design pattern
class imcomp_request_handler {
  boost::filesystem::path upload_dir_;
  boost::filesystem::path result_dir_;
  boost::filesystem::path asset_dir_;

  imcomp_request_handler() { };
  imcomp_request_handler(const imcomp_request_handler& sh) { };
  imcomp_request_handler* operator=(const imcomp_request_handler &) {
    return 0;
  }

  static imcomp_request_handler* imcomp_request_handler_;

  // _upload
  bool save_user_upload(const http_request& request, string& fid);
  bool transform_and_save_user_upload(const http_request& request, string& fid);
  bool transform_file(const http_request& request, string& fid);

  // _compare
  void register_images();

  // result
  bool has_invalid_char(const std::string s);
  bool load_file_contents(const boost::filesystem::path fn,
                          std::string& file_contents);

  public:
  static imcomp_request_handler* instance();

  void init(const boost::filesystem::path upload_dir,
            const boost::filesystem::path result_dir,
            const boost::filesystem::path asset_dir);
  void handle_http_request(const http_request& request, http_response& response);
};
#endif
