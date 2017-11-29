#include "imcomp/imcomp_request_handler.h"

imcomp_request_handler *imcomp_request_handler::imcomp_request_handler_ = NULL;

imcomp_request_handler* imcomp_request_handler::instance() {
  if ( !imcomp_request_handler_ ) {
    imcomp_request_handler_ = new imcomp_request_handler;
  }
  return imcomp_request_handler_;
}

void imcomp_request_handler::init(const boost::filesystem::path upload_dir, const boost::filesystem::path result_dir) {
  upload_dir_ = upload_dir;
  result_dir_ = result_dir;
  cout << "\nimcomp_request_handler::init() : initializing http request handler" << flush;
  cout << "\nimcomp_request_handler::init() : upload_dir=" << upload_dir_.string() << flush;
  cout << "\nimcomp_request_handler::init() : result_dir=" << result_dir_.string() << flush;

  for( auto it=imcomp::asset::files_.begin(); it!=imcomp::asset::files_.end(); it++ ) {
    cout << "\n\t" << it->first << " : " << it->second.length() << flush;
  }
}

void imcomp_request_handler::handle_http_request(const http_request& request, http_response& response) {
  cout << "\n" << request.method_ << " [" << request.uri_ << "]" << flush;
  response.set_status(200);
  if ( (request.uri_ == "/imcomp") || (request.uri_ == "/imcomp/")) {
    std::string asset_fn("/imcomp/index.html");
    auto asset_loc = imcomp::asset::files_.find(asset_fn);
    response.set_payload(asset_loc->second);
    return;
  }

  if ( (request.uri_ == "/imcomp/traherne") || (request.uri_ == "/imcomp/traherne/")) {
    std::string asset_fn("/imcomp/traherne/index.html");
    auto asset_loc = imcomp::asset::files_.find(asset_fn);
    response.set_payload(asset_loc->second);
    return;
  }

  if ( util::begins_with(request.uri_, "/imcomp/result/") ) {
    // serve dynamic resource from result_dir_
    // @todo
    response.set_status(400);
    return;
  }

  auto asset_found = imcomp::asset::files_.find(request.uri_);
  if ( asset_found != imcomp::asset::files_.end() && request.method_ == "GET" ) {
    response.set_payload( asset_found->second );
    return;
  }

  if ( request.uri_ == "/imcomp/_upload" && request.method_ == "POST" ) {
    // handle upload
    // @todo
  }

  if ( util::begins_with(request.uri_, "/imcomp/_compare") ) {
    // handle compare
    // @todo
  }

  response.set_status(400);
}

