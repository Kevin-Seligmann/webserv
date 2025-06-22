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
    transfer_encodings.clear();
    connections.clear();
    cookies.clear();
}

void FieldSection::put(std::string const & str, std::string const & value)
{
    if (fields.find(str) == fields.end())
        fields[str] = value;
    else
        fields[str] += ", " + value;
}

void FieldSection::print(std::ostream & os) const
{
    for (std::map<std::string, std::string>::const_iterator it = fields.begin(); it != fields.end(); it ++)
        os << "\t" << it->first << ": "  << it->second << std::endl;
    os << "\tHeader parsed data... "
    << " port: " << port
    << " host: " << host
    << " content-length: " << content_length;
    if (transfer_encodings.size() > 0)
    {
        os << "\n\ttransfer-encodings: ";
        for (std::vector<CommaSeparatedFieldValue>::const_iterator it = transfer_encodings.begin(); it != transfer_encodings.end(); it ++)
        {
            os << it->name << " ";
            for (std::vector<std::pair<std::string, std::string> >::const_iterator params = it->parameters.begin(); params != it->parameters.end(); params ++)
            {
                os << "param: " << params->first << "=" << params->second;
                if (params + 1 != it->parameters.end())
                    os << "; ";
            }
            if (it + 1 != transfer_encodings.end())
                os << ", ";
        }
    }
    if (cookies.size() > 0)
    {
        os << "\n\tCookies: ";
        for (std::map<std::string, std::string>::const_iterator it = cookies.begin(); it != cookies.end(); it ++)
        {
            os << it->first << "=" << it->second;
            std::map<std::string, std::string>::const_iterator next = it;
            next ++;
            if (next != cookies.end())
                os << "; ";
        }
    }
    os << std::endl;
}


void FieldSection::put_cookie(std::string::iterator name_start, std::string::iterator name_end, std::string::iterator value_start, std::string::iterator value_end)
{
    cookies[std::string(name_start, name_end)] = std::string(value_start, value_end);   
}

std::ostream & operator<<(std::ostream & os,  FieldSection const & header)
{
    header.print(os);
    return os;
}
