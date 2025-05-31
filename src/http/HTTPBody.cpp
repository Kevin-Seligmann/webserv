#include "HTTPBody.hpp"

HTTPBody::HTTPBody(){}

void HTTPBody::reset()
{
    raw = "";
}

void HTTPBody::put(std::string const & str)
{
    raw = str;
    // validate()
}

void HTTPBody::print(std::ostream & os) const
{
    os << raw;
}

std::ostream & operator<<(std::ostream & os,  HTTPBody const & line)
{
    line.print(os);
    return os;
}