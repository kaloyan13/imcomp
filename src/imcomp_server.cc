/** @file   imcomp_server.cc
 *  @brief  main entry point for the imcomp web demo server
 *
 *  @author Abhishek Dutta (adutta@robots.ox.ac.uk)
 *  @date   29 Nov. 2017
 */

#include <iostream>
#include <string>
#include <thread>

#include <boost/filesystem.hpp>

#include "imcomp_server_config.h"
#include "http_server/http_server.h"
#include "imcomp/imcomp_request_handler.h"

// create dir if not exists
bool mkdir_p(boost::filesystem::path p, bool verbose=false) {
  if ( !boost::filesystem::exists(p) ) {
    boost::filesystem::create_directories(p);
    if ( verbose ) {
      std::cout << "\nCreated directory : " << p.string() << std::flush;
    }
    return true;
  }
  return false;
}

int main(int argc, char** argv) {
  std::cout << IMCOMP_SERVER_NAME << " " 
            << IMCOMP_SERVER_VERSION_MAJOR << "."
            << IMCOMP_SERVER_VERSION_MINOR << "."
            << IMCOMP_SERVER_VERSION_PATCH << flush;
        
  std::cout << "\nAuthor: " 
            << IMCOMP_SERVER_AUTHOR_NAME << "<" 
            << IMCOMP_SERVER_AUTHOR_EMAIL << ">, "
            << IMCOMP_SERVER_FIRST_RELEASE << flush;

  if ( argc != 5 && argc != 6 ) {
    std::cout << "\nUsage: " << argv[0] << " hostname port thread_count application_data_dir | [upload_dir result_dir]\n" << std::flush;
    return 0;
  }

  std::string address = argv[1];
  std::string port    = argv[2];

  unsigned int thread_pool_size;
  std::stringstream s;
  s.clear(); s.str(argv[3]);
  s >> thread_pool_size;

  boost::filesystem::path upload_dir;
  boost::filesystem::path result_dir;
  if ( argc == 5 ) {
    boost::filesystem::path app_dir( argv[4] );
    upload_dir = app_dir / "upload";
    result_dir = app_dir / "result";
  }
  if ( argc == 6 ) {
    upload_dir = boost::filesystem::path(upload_dir);
    result_dir = boost::filesystem::path(result_dir);
  }

  mkdir_p(upload_dir);
  mkdir_p(result_dir);

  imcomp_request_handler::instance()->init(upload_dir, result_dir);

  http_server server(address, port, thread_pool_size, upload_dir, result_dir);
  server.start();

  // cleanup
  //boost::filesystem::remove_all( upload_dir );
  //boost::filesystem::remove_all( result_dir );
  return 0;
}
