#include "HTTPBody.hpp"

HTTPBody::HTTPBody(){}

void HTTPBody::reset()
{
    content = "";
}


void HTTPBody::print(std::ostream & os) const
{
    os << content;
}

std::ostream & operator<<(std::ostream & os,  HTTPBody const & line)
{
    line.print(os);
    return os;
}