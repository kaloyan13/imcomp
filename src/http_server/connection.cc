#include "http_server/connection.h"

const std::string connection::crlf = "\r\n";
const std::string connection::crlf2 = "\r\n\r\n";

const std::string connection::http_100 = "HTTP/1.1 100 Continue\r\n";
const std::string connection::http_200 = "HTTP/1.1 200 OK\r\n";
const std::string connection::http_301 = "HTTP/1.1 301 Moved Permanently\r\n";
const std::string connection::http_400 = "HTTP/1.1 400 Bad Request\r\n";
const std::string connection::app_namespace = "vise";

connection::connection(boost::asio::io_service& io_service): strand_( io_service ),
                                                            socket_( io_service )
{
  connection_id_ = boost::filesystem::unique_path("%%%%%%%%").string();
}

connection::~connection() {
}

boost::asio::ip::tcp::socket& connection::socket() {
  return socket_;
}

void connection::process_connection() {
  socket_.async_read_some(boost::asio::buffer( buffer_ ),
                          strand_.wrap( boost::bind(&connection::on_request_data, shared_from_this(),
                                                    boost::asio::placeholders::error,
                                                    boost::asio::placeholders::bytes_transferred
                                                    )
                                        )
                          );
}

void connection::on_request_data(const boost::system::error_code& e, std::size_t bytes_read) {
  //cout << "\nconnection::on_request_data() : bytes_read = " << bytes_read << flush;
  //cout << "\n  request_ = " << request_.print() << flush;
  std::string request_chunk( buffer_.data(), buffer_.data() + bytes_read );
  request_.parse(request_chunk);

  if ( request_.get_expect_100_continue_header() ) {
    response_http_100();
    request_.reset_expect_100_continue_header();
  }

  if ( request_.is_request_complete() ) {
    // process http request here
    //cout << "\n" << request_.print(false) << flush;
    //cout << "\npayload={" << request_.payload_.str() << "}" << flush;

    imcomp_request_handler::instance()->handle_http_request(request_, response_);
    send_response();
    return;

  } else {
    // fetch more chunks of http request to get the complete http request
    //cout << "\nWaiting for more chunks of http request ..." << flush;
    //cout << "\nhttp request so far = {\n" << request_chunk << "}" << std::flush;
    socket_.async_read_some(boost::asio::buffer( buffer_ ),
                            strand_.wrap( boost::bind(&connection::on_request_data, shared_from_this(),
                                                      boost::asio::placeholders::error,
                                                      boost::asio::placeholders::bytes_transferred
                                                      )
                                          )
                            );
  }
}

void connection::send_response() {
  std::ostream http_response( &response_buffer_ );
  http_response << response_.status_ << connection::crlf;

  for( auto it = response_.fields_.begin(); it != response_.fields_.end(); it++ ) {
    http_response << it->first << ": " << it->second << connection::crlf;
  }
  http_response << connection::crlf << response_.payload_ << flush;

  boost::asio::async_write(socket_, response_buffer_.data(),
                           strand_.wrap(boost::bind(&connection::on_response_write,
                                                    shared_from_this(),
                                                    boost::asio::placeholders::error
                                                    )
                                        )
                           );
}

void connection::response_http_100() {
  std::ostream http_response( &continue_response_buffer_ );
  http_response << connection::http_100;

  boost::asio::async_write(socket_, continue_response_buffer_.data(),
                           strand_.wrap(boost::bind(&connection::on_http_100_response_write,
                                                    shared_from_this(),
                                                    boost::asio::placeholders::error
                                                    )
                                        )
                           );
}

void connection::on_http_100_response_write(const boost::system::error_code& e) {
  if ( e ) {
    std::cerr << "\nfailed to send 100 continue response" << std::endl;
  }
}

void connection::on_response_write(const boost::system::error_code& e) {
  if ( e ) {
    std::cerr << "\nfailed to send http response" << std::endl;
  }

  close_connection(e);
}

void connection::close_connection(const boost::system::error_code& e) {
  boost::system::error_code ignored_err;
  socket_.shutdown( boost::asio::ip::tcp::socket::shutdown_both, ignored_err );
}
