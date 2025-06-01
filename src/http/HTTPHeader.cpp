#include "HTTPHeader.hpp"
#include "ParsingUtil.hpp"
#include "StringUtil.hpp"

HTTPHeader::HTTPHeader(){}

void HTTPHeader::reset()
{
    _raw = "";
}

void HTTPHeader::put(std::string const & str, std::string const & value)
{
    _raw += "\t" + str + "\n";
    //std::string::const_iterator it = str.begin();
    // while (*it != ':')
    //     it ++;
    // std::string name(str.begin(), it);
    // if (!parse::is_token(name))
    //     throw std::runtime_error("Invalid header name: " + name);
    // wss::to_upper(name);
}

void HTTPHeader::print(std::ostream & os) const
{
    os << _raw;
}

std::ostream & operator<<(std::ostream & os,  HTTPHeader const & header)
{
    header.print(os);
    return os;
}