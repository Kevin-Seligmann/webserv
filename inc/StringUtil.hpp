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
}