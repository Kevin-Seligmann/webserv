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
}

void URI::print(std::ostream & os) const
{
    os << "\n"
    << "\tSCHEMA: " << schema << "\n"
    << "\tHOST: " << host << "\n"
    << "\tPORT: " << port << "\n"
    << "\tPATH: " << path << "\n"
    << "\tQUERY: " << query << "\n"
    << "\tFRAGMENT: " << fragment
    ;
}

std::ostream & operator<<(std::ostream & os,  URI const & uri)
{
    uri.print(os);
    return os;
}