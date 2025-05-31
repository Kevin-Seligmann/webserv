#include "HTTPHeader.hpp"

HTTPHeader::HTTPHeader(){}

void HTTPHeader::reset()
{
    raw = "";
}

void HTTPHeader::put(std::string const & str)
{
    raw += "\t" + str + "\n";
    // validate()
}

void HTTPHeader::print(std::ostream & os) const
{
    os << raw;
}

std::ostream & operator<<(std::ostream & os,  HTTPHeader const & header)
{
    header.print(os);
    return os;
}