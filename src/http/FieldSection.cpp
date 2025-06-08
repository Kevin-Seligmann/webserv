#include "FieldSection.hpp"
#include "ParsingUtil.hpp"
#include "StringUtil.hpp"

FieldSection::FieldSection(){reset();}

void FieldSection::reset()
{
    port = -1;
    host = "";
    content_length = -1;
    fields.clear();
}

void FieldSection::put(std::string const & str, std::string const & value)
{
    fields[str].push_back(value);
}

void FieldSection::print(std::ostream & os) const
{
    for (std::map<std::string, std::vector<std::string> >::const_iterator it = fields.begin(); it != fields.end(); it ++)
    {
        os << "\t" << it->first << ": " ;
        for (std::vector<std::string>::const_iterator val = it->second.begin(); val != it->second.end(); val ++)
        {
            os << "\"" << *val << "\"";
            if (val + 1 != it->second.end())
                os << ", ";
        }
        os << std::endl;
    }
    os << "\tHeader parsed data... "
    << " port: " << port
    << " host: " << host
    << " content-length: " << content_length
    << std::endl;
}

std::ostream & operator<<(std::ostream & os,  FieldSection const & header)
{
    header.print(os);
    return os;
}