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
/*
  if ( (request.uri_ == "/imcomp") || (request.uri_ == "/imcomp/")) {
    std::string asset_fn("/imcomp/index.html");
    auto asset_loc = imcomp::asset::files_.find(asset_fn);
    response.set_payload(asset_loc->second);
    response.set_field("Content-Type", "text/html");
    return;
  }

  if ( (request.uri_ == "/imcomp/traherne") || (request.uri_ == "/imcomp/traherne/")) {
    std::string asset_fn("/imcomp/traherne/index.html");
    auto asset_loc = imcomp::asset::files_.find(asset_fn);
    response.set_payload(asset_loc->second);
    response.set_field("Content-Type", "text/html");
    return;
  }
*/

  if ( util::begins_with(request.uri_, "/imcomp/result/") ) {
    // serve dynamic resource from result_dir_
    // @todo
    string prefix = "/imcomp/result/";
    string res = request.uri_.substr(prefix.length());
    if(has_invalid_char(res)) {
      response.set_status(400);
    } else {
      boost::filesystem::path fn = result_dir_ / res;
      std::string file_contents;
      if( load_file_contents(fn, file_contents) ) {
        response.set_payload(file_contents);
        response.set_content_type_from_filename(fn.string());
      } else {
        response.set_status(400);
      }
    }
    return;
  }

  if ( util::begins_with(request.uri_, "/imcomp/traherne/") ) {
    // serve dynamic resource from result_dir_
    // @todo
    string prefix = "/imcomp/traherne/";
    string res = request.uri_.substr(prefix.length());
    
    if(has_invalid_char(res)) {
      response.set_status(400);
    } else {
      // @todo avoid static links
      boost::filesystem::path fn("/data/adutta/vggdemo/traherne/imcomp/asset/imcomp/traherne");
      fn = fn / res;
      std::string file_contents;
      if( load_file_contents(fn, file_contents) ) {
        response.set_payload(file_contents);
        response.set_content_type_from_filename(fn.string());
      } else {
        response.set_status(400);
      }
    }
    return;
  }

/*
  if ( request.method_ == "GET" && util::begins_with(request.uri_, "/imcomp/") ) {
    // serve dynamic resource from result_dir_
    // @todo
    string prefix = "/imcomp/";
    string res = request.uri_.substr(prefix.length());
    
    if(has_invalid_char(res)) {
      response.set_status(400);
    } else {
      boost::filesystem::path fn("/home/tlm/dev/imcomp/asset/imcomp");
      fn = fn / res;
      std::string file_contents;
      if( load_file_contents(fn, file_contents) ) {
        response.set_payload(file_contents);
        response.set_content_type_from_filename(fn.string());
      } else {
        response.set_status(400);
      }
    }
    return;
  }

  auto asset_found = imcomp::asset::files_.find(request.uri_);
  if ( asset_found != imcomp::asset::files_.end() && request.method_ == "GET" ) {
    response.set_payload( asset_found->second );
    response.set_content_type_from_filename(asset_found->first);
    return;
  }
*/

  if ( request.uri_ == "/imcomp/_upload" && request.method_ == "POST" ) {
    string fid;
    bool success = save_user_upload(request, fid);
    response.set_field("Content-Type", "application/json");
    if(success) {
      string payload = "{\"fid\":\"" + fid + "\"}";
      response.set_payload(payload);
      return;
    } else {
      string payload = "{\"status\":\"ERR\", \"description\":\"failed to save file\"}";
      response.set_payload(payload);
    }
    return;
  }

  if ( util::begins_with(request.uri_, "/imcomp/_compare") ) {
    map<string,string> uri_arg;
    bool success = request.parse_uri(uri_arg);
    //for( auto it=uri_arg.begin(); it!=uri_arg.end(); it++ ) {
      //cout << "\n" << it->first << ":" << it->second << flush;
    //}
    if(success) {
      string fid1 = uri_arg["file1"];
      string fid2 = uri_arg["file2"];
      string region1_str = uri_arg["region"];

      //cout << "\n  fid1=" << fid1 << flush;
      //cout << "\n  fid2=" << fid2 << flush;
      //cout << "\n  region1_str=" << region1_str << flush;

      unsigned int file1_region[4]; // x0, y0, x1, y1
      std::istringstream is(region1_str);
      char c = ',';
      is >> file1_region[0] >> c >> file1_region[1] >> c >> file1_region[2] >> c >> file1_region[3];

      string compare_id = boost::filesystem::unique_path("%%%%%%").string();

      boost::filesystem::path im1_fn = upload_dir_ / (fid1 + ".jpg");
      boost::filesystem::path im2_fn = upload_dir_ / (fid2 + ".jpg");
      boost::filesystem::path im1_out_fn = result_dir_ / (fid1 + "_" + compare_id + "_crop.jpg");
      boost::filesystem::path im2_out_fn = result_dir_ / (fid2 + "_" + compare_id + + "_crop.jpg");
      boost::filesystem::path im2_tx_fn  = result_dir_ / (fid2 + "_" + compare_id + + "_crop_tx.jpg");
      boost::filesystem::path diff_fn    = result_dir_ / (fid1 + "_" + fid2 + "_" + compare_id + "_diff.jpg");
      boost::filesystem::path overlap_fn    = result_dir_ / (fid1 + "_" + fid2 + "_" + compare_id + "_overlap.jpg");

      double h[9];
      uint32_t best_inliers_count = -1;
      homography eye;
/*
      registerImages::registerFromGuess( im1_fn.string().c_str(),
                                         im2_fn.string().c_str(),
                                         file1_region[0], file1_region[2], file1_region[1], file1_region[3],
                                         eye, best_inliers_count,
                                         im1_out_fn.string().c_str(),
                                         im2_out_fn.string().c_str(),
                                         im2_tx_fn.string().c_str(),
                                         diff_fn.string().c_str(),
                                         overlap_fn.string().c_str()
                                         );
*/

      vl_register_images::register_images( im1_fn.string().c_str(),
                                         im2_fn.string().c_str(),
                                         file1_region[0], file1_region[2], file1_region[1], file1_region[3],
                                         eye, best_inliers_count,
                                         im1_out_fn.string().c_str(),
                                         im2_out_fn.string().c_str(),
                                         im2_tx_fn.string().c_str(),
                                         diff_fn.string().c_str(),
                                         overlap_fn.string().c_str()
                                         );
      eye.exportToDoubleArray( h );

      std::ostringstream json;
      if ( best_inliers_count >= 9 ) {
        double file2_region[8];
        homography::affTransform(h, file1_region[0], file1_region[1], file2_region[0], file2_region[1]);
        homography::affTransform(h, file1_region[0], file1_region[3], file2_region[2], file2_region[3]);
        homography::affTransform(h, file1_region[2], file1_region[3], file2_region[4], file2_region[5]);
        homography::affTransform(h, file1_region[2], file1_region[1], file2_region[6], file2_region[7]);
        //std::cout << "\nfile2_region = " << file2_region[0] << "," << file2_region[1] << "," << file2_region[2] << "," << im2_region[3] << std::flush;

        json << "{\"IMAGE_HOMOGRAPHY\":[{"
             << "\"status\": \"OK\","
             << "\"status_message\": \"\","
             << "\"best_inliers_count\": \"" << best_inliers_count << "\","
             << "\"homography\": ["
             << h[0] << ", " << h[1] << ", " << h[2] << ", "
             << h[3] << ", " << h[4] << ", " << h[5] << ", "
             << h[6] << ", " << h[7] << ", " << h[8] << "],"
             << "\"file2_region\": [" << file2_region[0] << "," << file2_region[1] << "," << file2_region[2] << "," << file2_region[3] << ","
             << file2_region[4] << "," << file2_region[5] << "," << file2_region[6] << "," << file2_region[7] << "],"
             << "\"file1_crop\":\"/imcomp/result/" + im1_out_fn.filename().string() << "\","
             << "\"file2_crop\":\"/imcomp/result/" + im2_out_fn.filename().string() << "\","
             << "\"file2_crop_tx\":\"/imcomp/result/" + im2_tx_fn.filename().string() << "\","
             << "\"file1_file2_overlap\":\"/imcomp/result/" + overlap_fn.filename().string() << "\","
             << "\"file1_file2_diff\":\"/imcomp/result/" + diff_fn.filename().string() << "\"";
      } else {
        json << "{\"IMAGE_HOMOGRAPHY\":[{"
             << "\"status\": \"ERR\","
//             << "\"status_message\": \"Could not match sufficient feature points (best inliers count = "
             << "\"status_message\": \"Could not match sufficient feature points (matched features = "
             << best_inliers_count << ")\","
             << "\"best_inliers_count\": \"" << best_inliers_count << "\","
             << "\"homography\": []";
      }
      json << "}]}";
      response.set_field("Content-Type", "application/json");
      response.set_payload(json.str());
    } else {
      cout << "\nfailed to parse arguments from uri" << flush;
      response.set_status(400);
    }
    return;
  }

  response.set_status(400);
}

bool imcomp_request_handler::save_user_upload(const http_request& request, string& fid) {
  try {
    const std::string img_data = request.payload_.str();
    Magick::Blob blob(img_data.c_str(), img_data.length());

    if ( blob.length() ) {
      Magick::Image im(blob);

      // to ensure thread safety, we initialize it every time
      boost::uuids::random_generator uuid_generator;
      boost::uuids::uuid request_uuid = uuid_generator();
      fid = boost::uuids::to_string(request_uuid);

      boost::filesystem::path fn = upload_dir_ / ( fid + ".jpg");
      im.magick("JPEG");
      im.colorSpace(Magick::sRGBColorspace);
      im.write(fn.string());
      std::clog << " : " << fn.string() << " (" << blob.length() << " bytes)" << std::flush;
      return true;
    } else {
      return false;
    }
  } catch( std::exception &e ) {
    std::clog << "\n  imcomp_request_handler::save_user_upload() : error saving file";
    std::clog << e.what() << std::flush;
    return false;
  }
}


bool imcomp_request_handler::has_invalid_char(const std::string s) {
  if ( s.find('!') != std::string::npos )
    return true;
  if ( s.find('"') != std::string::npos )
    return true;
  if ( s.find('$') != std::string::npos )
    return true;
  if ( s.find('%') != std::string::npos )
    return true;
  if ( s.find('^') != std::string::npos )
    return true;
  if ( s.find('&') != std::string::npos )
    return true;
  if ( s.find('*') != std::string::npos )
    return true;
  if ( s.find('!') != std::string::npos )
    return true;
  if ( s.find('@') != std::string::npos )
    return true;
  if ( s.find('~') != std::string::npos )
    return true;
  if ( s.find('?') != std::string::npos )
    return true;
  if ( s.find('/') != std::string::npos )
    return true;
  if ( s.find('\\') != std::string::npos )
    return true;

  return false;
}

bool imcomp_request_handler::load_file_contents(const boost::filesystem::path fn, std::string& file_contents) {
  if( !boost::filesystem::exists(fn) ) {
    return false;
  }

  try {
    std::ifstream f;
    f.open(fn.string().c_str(), std::ios::in | std::ios::binary);
    f.seekg(0, std::ios::end);
    file_contents.clear();
    file_contents.reserve( f.tellg() );
    f.seekg(0, std::ios::beg);
    file_contents.assign( (std::istreambuf_iterator<char>(f)),
                          std::istreambuf_iterator<char>() );
    f.close();
    return true;
  } catch(std::exception &e) {
    return false;
  }
}
