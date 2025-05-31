#include "HTTPFirstLine.hpp"

HTTPFirstLine::HTTPFirstLine(){}

void HTTPFirstLine::reset()
{
    raw = "";
}

void HTTPFirstLine::put(std::string const & str)
{
    raw = str;
    // validate()
}

void HTTPFirstLine::print(std::ostream & os) const
{
    os << raw;
}

std::ostream & operator<<(std::ostream & os,  HTTPFirstLine const & line)
{
    line.print(os);
    return os;
}