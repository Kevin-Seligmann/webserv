#include "URI.hpp"

URI::URI()
{reset();}

void URI::reset()
{
    host = "";
    path = "";
    query = "";
    fragment = "";
    port = -1;
    schema = "";
    length = 0;
}

void URI::print(std::ostream & os) const
{
    os << "\n"
    << "\tSCHEMA: " << schema << ", "
    << "HOST: " << host << ", "
    << "PORT: " << port << ", "
    << "PATH: " << path << ", "
    << "QUERY: " << query << ", "
    << "FRAGMENT: " << fragment
    ;
}

std::ostream & operator<<(std::ostream & os,  URI const & uri)
{
    uri.print(os);
    return os;
}