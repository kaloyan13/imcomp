#include "imcomp/imcomp_request_handler.h"

imcomp_request_handler *imcomp_request_handler::imcomp_request_handler_ = NULL;

imcomp_request_handler* imcomp_request_handler::instance() {
  if ( !imcomp_request_handler_ ) {
    imcomp_request_handler_ = new imcomp_request_handler;
  }
  return imcomp_request_handler_;
}

void imcomp_request_handler::init(const boost::filesystem::path upload_dir,
                                  const boost::filesystem::path result_dir,
                                  const boost::filesystem::path asset_dir) {
  upload_dir_ = upload_dir;
  result_dir_ = result_dir;
  asset_dir_ = asset_dir;
  cout << "\nImageMagick Magick++ quantum depth = " << MAGICKCORE_QUANTUM_DEPTH << flush;
  cout << "\nimcomp_request_handler::init() : initializing http request handler" << flush;
  cout << "\nimcomp_request_handler::init() : upload_dir=" << upload_dir_.string() << flush;
  cout << "\nimcomp_request_handler::init() : result_dir=" << result_dir_.string() << flush;
  cout << "\nimcomp_request_handler::init() : asset_dir=" << asset_dir_.string() << flush;
}

void imcomp_request_handler::handle_http_request(const http_request& request, http_response& response) {
  cout << "\n" << request.method_ << " [" << request.uri_ << "]" << flush;
  response.set_status(200);

  if ( util::begins_with(request.uri_, "/imcomp/static/result/") && request.method_ == "GET") {
    string prefix = "/imcomp/static/result/";
    string static_res = request.uri_.substr(prefix.length());
    boost::filesystem::path static_res_path( static_res );
    static_res_path = static_res_path.lexically_normal();
    boost::filesystem::path fn = result_dir_ / static_res_path;
    std::string file_contents;
    if( load_file_contents(fn, file_contents) ) {
      response.set_payload(file_contents);
      response.set_content_type_from_filename(fn.string());
    } else {
      response.set_status(400);
    }

    return;
  }

  if ( util::begins_with(request.uri_, "/imcomp/") && request.method_ == "GET") {
    // serve application assets (css, html, js, etc)
    string prefix = "/imcomp/";
    string static_res = request.uri_.substr(prefix.length());
    boost::filesystem::path static_res_path( static_res );
    static_res_path = static_res_path.lexically_normal();
    boost::filesystem::path fn = asset_dir_ / static_res_path;
    std::string file_contents;
    if( load_file_contents(fn, file_contents) ) {
      response.set_payload(file_contents);
      response.set_content_type_from_filename(fn.string());
    } else {
      response.set_status(400);
    }
    return;
  }

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

  if ( util::begins_with(request.uri_, "/imcomp/_compare") && request.method_ == "POST" ) {
    map<string,string> uri_arg;
    bool success = request.parse_uri(uri_arg);
    //for( auto it=uri_arg.begin(); it!=uri_arg.end(); it++ ) {
      //cout << "\n" << it->first << ":" << it->second << flush;
    //}
    if(success) {
      string fid1 = uri_arg["file1"];
      string fid2 = uri_arg["file2"];
      string region1_str = uri_arg["region"];
      string algname = "default";
      if(uri_arg.count("algname")) {
        algname = uri_arg["algname"];
      }

      //cout << "\n  fid1=" << fid1 << flush;
      //cout << "\n  fid2=" << fid2 << flush;
      //cout << "\n  region1_str=" << region1_str << flush;
      //cout << "\n  algname=" << algname << flush;

      unsigned int file1_region[4]; // x0, y0, x1, y1
      std::istringstream is(region1_str);
      char c = ',';
      is >> file1_region[0] >> c >> file1_region[1] >> c >> file1_region[2] >> c >> file1_region[3];

      string compare_id = boost::filesystem::unique_path("%%%%%%").string();

      boost::filesystem::path im1_fn = upload_dir_ / (fid1 + ".jpg");
      boost::filesystem::path im2_fn = upload_dir_ / (fid2 + ".jpg");
      boost::filesystem::path im1_crop_fn = result_dir_ / (fid1 + "_" + compare_id + "_crop.jpg");
      boost::filesystem::path im2_crop_fn = result_dir_ / (fid2 + "_" + compare_id + + "_crop.jpg");
      boost::filesystem::path im2_tx_fn  = result_dir_ / (fid2 + "_" + compare_id + + "_crop_tx.jpg");
      boost::filesystem::path diff_fn    = result_dir_ / (fid1 + "_" + fid2 + "_" + compare_id + "_diff.jpg");
      boost::filesystem::path overlap_fn    = result_dir_ / (fid1 + "_" + fid2 + "_" + compare_id + "_overlap.jpg");

      size_t fp_match_count = -1;
      MatrixXd H(3,3);
      bool success = false;
      if( algname == "robust_ransac_tps" ) {
        imreg_sift::robust_ransac_tps(im1_fn.string().c_str(),
                                      im2_fn.string().c_str(),
                                      file1_region[0], file1_region[2], file1_region[1], file1_region[3],
                                      H, 
                                      fp_match_count,
                                      im1_crop_fn.string().c_str(),
                                      im2_crop_fn.string().c_str(),
                                      im2_tx_fn.string().c_str(),
                                      diff_fn.string().c_str(),
                                      overlap_fn.string().c_str(),
                                      success);
      } else {
        imreg_sift::ransac_dlt(im1_fn.string().c_str(),
                                im2_fn.string().c_str(),
                                file1_region[0], file1_region[2], file1_region[1], file1_region[3],
                                H, 
                                fp_match_count,
                                im1_crop_fn.string().c_str(),
                                im2_crop_fn.string().c_str(),
                                im2_tx_fn.string().c_str(),
                                diff_fn.string().c_str(),
                                overlap_fn.string().c_str(),
                                success);
      }

      std::ostringstream json;
      if ( success ) {
        //std::cout << "\nfile2_region = " << file2_region[0] << "," << file2_region[1] << "," << file2_region[2] << "," << im2_region[3] << std::flush;

        json << "{\"IMAGE_HOMOGRAPHY\":[{"
             << "\"status\": \"OK\","
             << "\"status_message\": \"\","
             << "\"fp_match_count\": \"" << fp_match_count << "\","
             << "\"homography\": ["
             << H(0,0) << ", " << H(0,1) << ", " << H(0,2) << ", "
             << H(1,0) << ", " << H(1,1) << ", " << H(1,2) << ", "
             << H(2,0) << ", " << H(2,1) << ", " << H(2,2) << "],"
             << "\"file1_crop\":\"/imcomp/static/result/" + im1_crop_fn.filename().string() << "\","
             << "\"file2_crop\":\"/imcomp/static/result/" + im2_crop_fn.filename().string() << "\","
             << "\"file2_crop_tx\":\"/imcomp/static/result/" + im2_tx_fn.filename().string() << "\","
             << "\"file1_file2_overlap\":\"/imcomp/static/result/" + overlap_fn.filename().string() << "\","
             << "\"file1_file2_diff\":\"/imcomp/static/result/" + diff_fn.filename().string() << "\"";
      } else {
        json << "{\"IMAGE_HOMOGRAPHY\":[{"
             << "\"status\": \"ERR\","
//             << "\"status_message\": \"Could not match sufficient feature points (best inliers count = "
             << "\"status_message\": \"Could not match sufficient feature points (matched features = "
             << fp_match_count << ")\","
             << "\"fp_match_count\": \"" << fp_match_count << "\","
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
  cout << "\nA" << flush;

  try {
    cout << "\nB" << flush;
    const std::string img_data = request.payload_.str();
    cout << "\nimg_data = {" << img_data.substr(0,10) << "}" << flush;

    Magick::Blob blob(img_data.c_str(), img_data.length());
    cout << "\nD" << flush;

    if ( blob.length() ) {
      cout << "\nE" << flush;
      Magick::Image im(blob);
      
      cout << "\nF" << flush;

      // to ensure thread safety, we initialize it every time
      boost::uuids::random_generator uuid_generator;
      boost::uuids::uuid request_uuid = uuid_generator();
      fid = boost::uuids::to_string(request_uuid);
      
      cout << "\nF1" << flush;
      boost::filesystem::path fn = upload_dir_ / ( fid + ".jpg");
      cout << "\nF2" << flush;
      //im.magick("JPEG");
      cout << "\nF3" << flush;
      im.colorSpace(Magick::sRGBColorspace);
      cout << "\nG" << flush;
      im.write(fn.string());
      cout << "\nH" << flush;
      std::clog << " : " << fn.string() << " (" << blob.length() << " bytes)" << std::flush;
      cout << "\nfid = " << fid << flush;
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
  if ( s.find('.') != std::string::npos )
    return true;

  /*  if ( s.find('/') != std::string::npos )
    return true;
  if ( s.find('\\') != std::string::npos )
    return true;
  */

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
