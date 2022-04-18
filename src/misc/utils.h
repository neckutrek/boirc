#include <iostream>
#include <boost/asio.hpp>

std::ostream& operator<<(std::ostream& os, const boost::asio::ip::tcp::endpoint& endpoint);
