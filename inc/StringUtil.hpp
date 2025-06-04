#pragma once

#include <string>
#include <cstring>
#include "ParsingUtil.hpp"

namespace wss 
{
    void to_upper(std::string &);
    void to_lower(std::string &);
    void trim(std::string &);
    std::string::const_iterator skip_ascii_whitespace(std::string::const_iterator begin, std::string::const_iterator end);
    std::string::const_iterator skip_uri_token(std::string::const_iterator begin, std::string::const_iterator end);
    std::string::const_iterator skip_http_token(std::string::const_iterator begin, std::string::const_iterator end);
    std::string::const_iterator skip_protocol_token(std::string::const_iterator begin, std::string::const_iterator end);
    std::string::const_iterator skip_absolute_path(std::string::const_iterator begin, std::string::const_iterator end); // Any / and segments
    std::string::const_iterator skip_host_token(std::string::const_iterator begin, std::string::const_iterator end);
    std::string::const_iterator skip_port_token(std::string::const_iterator begin, std::string::const_iterator end);
    std::string::const_iterator skip_path_rootless(std::string::const_iterator begin, std::string::const_iterator end);
    std::string::const_iterator skip_query_token(std::string::const_iterator begin, std::string::const_iterator end);
    std::string::const_iterator skip_fragment_token(std::string::const_iterator begin, std::string::const_iterator end);
    std::string::const_iterator skip_schema_token(std::string::const_iterator begin, std::string::const_iterator end);
}

