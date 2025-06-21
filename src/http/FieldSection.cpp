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
    os << std::endl;
}

std::ostream & operator<<(std::ostream & os,  FieldSection const & header)
{
    header.print(os);
    return os;
}