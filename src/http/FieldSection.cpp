#include "FieldSection.hpp"
#include "ParsingUtil.hpp"
#include "StringUtil.hpp"

FieldSection::FieldSection(){_raw = "";}

void FieldSection::reset()
{
    _raw = "";
}

void FieldSection::put(std::string const & str, std::string const & value)
{
    _raw += "name: " + str + ", value: " + value;
    //std::string::const_iterator it = str.begin();
    // while (*it != ':')
    //     it ++;
    // std::string name(str.begin(), it);
    // if (!parse::is_token(name))
    //     throw std::runtime_error("Invalid header name: " + name);
    // wss::to_upper(name);
}

void FieldSection::print(std::ostream & os) const
{
    os << _raw;
}

std::ostream & operator<<(std::ostream & os,  FieldSection const & header)
{
    header.print(os);
    return os;
}