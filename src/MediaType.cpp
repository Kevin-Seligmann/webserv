#include "MediaType.hpp"

const std::string MediaType::TYPES_PATH = "./conf/media/types.csv";
t_mime_conf MediaType::ACCEPTED_TYPES;

void MediaType::load_types()
{
    std::ifstream in;
    in.exceptions(std::ifstream::badbit);

    try
    {
        in.open(TYPES_PATH.c_str());
        
        std::string line;
        std::getline(in, line); // Skip header

        while (!in.eof() && std::getline(in, line))
        {
            std::stringstream ss(line);
            ss.exceptions(std::ifstream::badbit);

            std::string type, subtype;
            std::getline(ss, type, ',');
            std::getline(ss, subtype, ',');
            wss::trim(type);
            wss::trim(subtype);

            std::vector<std::string> exts;
            std::string ext;
            while (!ss.eof() && std::getline(ss, ext, ','))
            {
                wss::trim(ext);
                exts.push_back(ext);
            }
            ACCEPTED_TYPES.push_back(std::make_pair(MediaType(type, subtype), exts));
        }
    } catch (const std::ifstream::failure& e)
    {
        Logger::getInstance().warning("Error reading MIME configuration. The server would not accept any Content-Types and 'native' POST requests");
        Logger::getInstance() << e.what();
        ACCEPTED_TYPES.clear();
        return ;
    }
}

MediaType::MediaType(){}

MediaType::MediaType(std::string type, std::string subtype):type(type), subtype(subtype){}

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