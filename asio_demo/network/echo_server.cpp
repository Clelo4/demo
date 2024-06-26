#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/system/detail/error_code.hpp>
#include <iostream>

void handle_client(std::shared_ptr<boost::asio::ip::tcp::socket> socket_ptr) {
  boost::asio::streambuf buffer;
  boost::system::error_code error;

  boost::asio::read_until(*socket_ptr, buffer, '\n', error);
  if (error) {
    std::cerr << "Error reading data: " << error.message() << std::endl;
    return;
  }

  // Echo the data back to the client
  std::string message = boost::asio::buffer_cast<const char *>(buffer.data());
  boost::asio::write(*socket_ptr, boost::asio::buffer(message), error);
  if (error) {
    std::cerr << "Error writing data: " << error.message() << std::endl;
  }
}

int main() {
  boost::asio::io_context io_context;
  boost::asio::ip::tcp::acceptor acceptor(
      io_context,
      boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 8888));

  std::cout << "TCP Echo Server started. Listening on port 8888." << std::endl;

  while (true) {
    std::shared_ptr<boost::asio::ip::tcp::socket> socket_ptr = std::make_shared<boost::asio::ip::tcp::socket>(io_context);
    acceptor.accept(*socket_ptr);

    std::cout << "New connection from: " << socket_ptr->remote_endpoint()
              << std::endl;

    // Handle the client in a separate thread
    std::thread(handle_client, socket_ptr).detach();
  }
  return 0;
}