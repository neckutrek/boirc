#include "utils.h"

std::ostream& operator<<(std::ostream& os, const boost::asio::ip::tcp::endpoint& endpoint)
{
  os << endpoint.address().to_string() << ":"
     << endpoint.port() << " ("
     << endpoint.protocol().family() << ", "
     << endpoint.protocol().type() << ")";
  return os;
}