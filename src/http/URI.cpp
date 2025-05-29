#include "URI.hpp"

URI::URI()
:raw(""),
host(""),
port(-1)
{}

void URI::reset()
{
    raw = "";
    host = "";
    port = -1;
}

void URI::print(std::ostream & os) const
{
    if (raw == "")
        os << "Empty URI";
    else
        os << raw;
}

std::ostream & operator<<(std::ostream & os,  URI const & uri)
{
    uri.print(os);
    return os;
}