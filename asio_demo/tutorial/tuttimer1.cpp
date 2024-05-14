#include <boost/asio.hpp>
#include <iostream>

int main() {
  boost::asio::io_context io;
  boost::asio::steady_timer t(io, boost::asio::chrono::microseconds(1000));
  t.wait();
  std::cout << "Hello, world!" << std::endl;

  return 0;
}