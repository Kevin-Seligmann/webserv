#include "MediaType.hpp"

std::ostream & operator<<(std::ostream & os,  MediaType const & media)
{
    os << media.type << "/" << media.subtype << "; ";
    for (std::vector<std::pair<std::string, std::string> >::const_iterator params = media.parameters.begin(); params != media.parameters.end(); params ++)
    {
        os << params->first << "=" << params->second;
        if (params + 1 != media.parameters.end())
            os << "; ";
    }
    return os;
}

void MediaType::reset()
{
    type = "";
    subtype = "";
    parameters.clear();
}