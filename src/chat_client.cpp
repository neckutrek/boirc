#include <cstdlib>
#include <queue>
#include <iostream>
#include <thread>
#include <utility>

#include <boost/asio.hpp>

#include "chat_message.hpp"

using boost::asio::ip::tcp;

typedef boost::asio::buffers_iterator<boost::asio::const_buffers_1> iterator;

class chat_client
{
public:
  chat_client(boost::asio::io_context& io_context,
      const tcp::resolver::results_type& endpoints)
    : io_context_(io_context),
      socket_(io_context)
  {
    do_connect(endpoints);
  }

  void write(const std::string& msg)
  {
    boost::asio::post(io_context_,
        [this, msg]()
        {
          bool write_in_progress = !write_msgs_.empty();
          write_msgs_.push(msg);
          if (!write_in_progress)
          {
            do_write();
          }
        });
  }

  void close()
  {
    boost::asio::post(io_context_, [this]() { socket_.close(); });
  }

private:
  void do_connect(const tcp::resolver::results_type& endpoints)
  {
    boost::asio::async_connect(socket_, endpoints,
        [this](boost::system::error_code ec, tcp::endpoint endpoint)
        {
          if (!ec)
          {
            std::cout << "Connected to: " << endpoint << std::endl;
            do_read_irc();
          }
          else
          {
            std::cerr << "Failed to connect to endpoint." << std::endl;
          }
        });
  }

  static std::pair<iterator, bool> is_irc_msg(iterator begin, iterator end)
  {
    {
      iterator it1 = begin;
      int n=0;
      while (it1 != end)
      {
        it1++;
        n++;
      }
      if (n < 2)
      {
        return std::make_pair(begin, false);
      }
    }

    iterator it = begin;
    bool result = false;
    while (!result && it+1 != end)
    {
      if (*it == (char)(0x0d) && *(it+1) == (char)(0x0a))
      {
        result = true;
      }
      else
      {
        it++;
      }
    }

    if (result)
    {
      return std::make_pair(it, true);
    }
    return std::make_pair(begin, false);
  };

  void do_read_irc()
  {
    boost::asio::async_read_until(
      socket_,
      b_,
      is_irc_msg,
      [this](boost::system::error_code ec, std::size_t length)
      {
        if (!ec)
        {
          std::istream is(&b_);
          std::string line;
          std::getline(is, line);
          std::cout << line << std::endl;
          do_read_irc();
        }
        else
        {
          socket_.close();
          std::cerr << "Closing socket (0)" << std::endl;
        }
      });
  }

  void do_read_header()
  {
    boost::asio::async_read(socket_,
        boost::asio::buffer(read_msg_.data(), chat_message::header_length),
        [this](boost::system::error_code ec, std::size_t /*length*/)
        {
          if (!ec && read_msg_.decode_header())
          {
            do_read_body();
          }
          else
          {
            socket_.close();
            std::cerr << "Closing socket (1)" << std::endl;
          }
        });
  }

  void do_read_body()
  {
    boost::asio::async_read(socket_,
        boost::asio::buffer(read_msg_.body(), read_msg_.body_length()),
        [this](boost::system::error_code ec, std::size_t length)
        {
          if (!ec)
          {
            if (length > 0)
            {
              std::cout.write(read_msg_.body(), read_msg_.body_length());
              std::cout << std::endl;
            }
            do_read_header();
          }
          else
          {
            socket_.close();
            std::cerr << "Closing socket (2)" << std::endl;
          }
        });
  }

  void do_write()
  {
    write_buff_.consume(write_buff_.size()); // clear the buffer
    std::ostream os{&write_buff_};
    os << write_msgs_.front();

    boost::asio::async_write(socket_,
        write_buff_,
        [this](boost::system::error_code ec, std::size_t length)
        {
          if (!ec)
          {
            write_msgs_.pop();
            if (!write_msgs_.empty())
            {
              do_write();
            }
          }
          else
          {
            socket_.close();
            std::cerr << "Closing socket (3)" << std::endl;
          }
        });
  }

private:
  boost::asio::io_context& io_context_;
  tcp::socket socket_;
  chat_message read_msg_;
  std::queue<std::string> write_msgs_;
  std::mutex m_;

  boost::asio::streambuf b_;
  boost::asio::streambuf write_buff_;

};

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 3)
    {
      std::cerr << "Usage: chat_client <host> <port>\n";
      return 1;
    }

    boost::asio::io_context io_context;

    tcp::resolver resolver(io_context);
    auto endpoints = resolver.resolve(argv[1], argv[2]);
    chat_client c(io_context, endpoints);

    std::thread t([&io_context](){ io_context.run(); });

    std::string line;
    while (std::getline(std::cin, line))
    {
      std::cout << "Input: " << line << std::endl;
      c.write(line);
    }

    c.close();
    t.join();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}