#include "HTTPHeader.hpp"

HTTPHeader::HTTPHeader(){}

void HTTPHeader::reset()
{
    raw_header = "";
    name = "";
    values.clear();
}

void HTTPHeader::print(std::ostream & os) const
{
    if (raw_header == "")
    {
        os << "No header";
    }
    else 
    {
        os << raw_header;
    }
}

std::ostream & operator<<(std::ostream & os,  HTTPHeader const & header)
{
    header.print(os);
    return os;
}