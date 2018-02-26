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
#include <Magick++.h>            // to transform images

#include "imcomp_server_config.h"
#include "http_server/http_server.h"
#include "imcomp/imcomp_request_handler.h"

int main(int argc, char** argv) {
  Magick::InitializeMagick(argv[0]);
  //std::cout << "\nMagick::InitializeMagick(*argv) : argv[0] = [" << argv[0] << "]" << std::endl;
  
  /*
  Magick::InitializeMagick("C:\\Users\\tlm\\dev\\imcomp\\bin\\Release");

  // for debug
  cout << "\nWriting test image ..." << flush;
  try {
    Magick::Image im( "100x100", "red");
    std::string format("JPEG");
    //std::string filename("C:\\Users\\tlm\\dev\\imcomp\\bin\\Release\\test.jpg");
    im.magick(format);
    //im.write(filename);
  } catch( std::exception &e ) {
    std::cout << "\n New Exception has occured! " << endl;
    std::cout << e.what() << std::flush;
  }
  cout << "\ndone" << flush;
  
  return 0;
  */
  std::cout << IMCOMP_SERVER_NAME << " "
            << IMCOMP_SERVER_VERSION_MAJOR << "."
            << IMCOMP_SERVER_VERSION_MINOR << "."
            << IMCOMP_SERVER_VERSION_PATCH << flush;

  std::cout << "\nAuthor: "
            << IMCOMP_SERVER_AUTHOR_NAME << "<"
            << IMCOMP_SERVER_AUTHOR_EMAIL << ">, "
            << IMCOMP_SERVER_CURRENT_RELEASE_DATE << flush;

  if ( argc != 6 && argc != 7 ) {
    std::cout << "\nUsage: " << argv[0] << " hostname port thread_count asset_dir [application_data_dir | [upload_dir result_dir] ]\n" << std::flush;
    return 0;
  }

  std::string address = argv[1];
  std::string port    = argv[2];
  boost::filesystem::path asset_dir(argv[4]);

  unsigned int thread_pool_size;
  std::stringstream s;
  s.clear(); s.str(argv[3]);
  s >> thread_pool_size;


  boost::filesystem::path upload_dir;
  boost::filesystem::path result_dir;
  if ( argc == 6 ) {
    boost::filesystem::path app_dir( argv[5] );
    upload_dir = app_dir / "upload";
    result_dir = app_dir / "result";
  }
  if ( argc == 7 ) {
    upload_dir = boost::filesystem::path(argv[5]);
    result_dir = boost::filesystem::path(argv[6]);
  }

  if ( !boost::filesystem::exists(upload_dir) ) {
    boost::filesystem::create_directories(upload_dir);
  }
  if ( !boost::filesystem::exists(result_dir) ) {
    boost::filesystem::create_directories(result_dir);
  }

  imcomp_request_handler::instance()->init(upload_dir, result_dir, asset_dir);

  http_server server(address, port, thread_pool_size);
  server.start();

  // cleanup
  //boost::filesystem::remove_all( upload_dir );
  //boost::filesystem::remove_all( result_dir );
  return 0;
}
