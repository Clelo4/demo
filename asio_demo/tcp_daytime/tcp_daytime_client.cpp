#include <array>
#include <boost/asio.hpp>
#include <iostream>

using boost::asio::ip::tcp;

int main(int argc, char *argv[]) {
  try {
    std::string host;
    if (argc != 2) {
      std::cerr << "Usage: client <host>" << std::endl;
      return 1;
    } else {
      host = argv[1];
    }

    boost::asio::io_context io_context;

    tcp::resolver resolver(io_context);

    // Resolve the host name (e.g., www.example.com) to a list of endpoints
    tcp::resolver::results_type endpoints =
        resolver.resolve("www.example.com", "http");

    auto res = endpoints->endpoint().address();

    // 定义服务器端点
    tcp::endpoint server_endpoint(boost::asio::ip::address::from_string(host),
                                  13);
    // 替换 "127.0.0.1" 和 8080 为服务器 IP 地址和端口号

    auto client_endpoint = tcp::endpoint(tcp::v4(), 9091);

    tcp::socket socket(io_context);

    socket.open(tcp::v4());
    socket.bind(client_endpoint);
    socket.connect(server_endpoint);

    // 设置地址和端口复用选项
    boost::asio::socket_base::reuse_address option(true);
    socket.set_option(option);

    std::cout << "Client is bound to port " << client_endpoint.port()
              << std::endl;

    for (;;) {
      std::array<char, 128> buf{};
      boost::system::error_code error;

      auto len = socket.read_some(boost::asio::buffer(buf), error);

      if (error == boost::asio::error::eof)
        break; // Connection closed cleanly by peer.
      else if (error)
        throw boost::system::system_error(error); // Some other error.

      std::cout.write(buf.data(), len);
      socket.close();
      break;
    }
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
