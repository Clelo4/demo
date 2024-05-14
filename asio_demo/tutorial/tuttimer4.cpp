#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <iostream>

class Printer {
public:
  Printer(boost::asio::io_context& io): timer_(io, boost::asio::chrono::seconds(1)) {
    timer_.async_wait(boost::bind(&Printer::print, this));
  }
  ~Printer() { std::cout << "Final count is" << count_ << std::endl; }

  void print() {
    if (count_ < 5) {
      std::cout << count_ << std::endl;
      ++count_;
      timer_.expires_at(timer_.expiry() + boost::asio::chrono::seconds(1));
      timer_.async_wait(boost::bind(&Printer::print, this));
    }
  }

private:
  int count_;
  boost::asio::steady_timer timer_;
};

void print(const boost::system::error_code &, boost::asio::steady_timer *t,
           int *count) {
  if (*count < 5) {
    std::cout << *count << std::endl;
    ++(*count);
    t->expires_at(t->expiry() + boost::asio::chrono::seconds(1));
    t->async_wait(
        boost::bind(print, boost::asio::placeholders::error, t, count));
  }
}

int main() {
  boost::asio::io_context io;
  Printer t(io);
  io.run();

  return 0;
}