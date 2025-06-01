#include "StringUtil.hpp"

void wss::to_upper(std::string & s)
{
    for(std::string::iterator it = s.begin(); it != s.end(); it ++)
        *it = std::toupper(*it);
}
void wss::to_lower(std::string & s)
{
    for(std::string::iterator it = s.begin(); it != s.end(); it ++)
        *it = std::tolower(*it);
}

void wss::trim(std::string & s)
{
    if (s.empty())
        return;

    std::string::iterator begin = s.begin();
    std::string::iterator end = s.end() - 1;

    while (begin != s.end() && parse::is_whitespace(*begin))
        begin++;
    while (end >= begin && parse::is_whitespace(*end))
        end--;
    s = std::string(begin, end + 1);
}

std::string::const_iterator wss::skip_ascii_whitespace(std::string::const_iterator begin, std::string::const_iterator end)
{
    while(begin != end && strchr(" \t\v\r\f" ,*begin)) begin ++; 
    return begin;
}

std::string::const_iterator wss::skip_http_token(std::string::const_iterator begin, std::string::const_iterator end)
{
    while(begin != end && parse::is_token_char(*begin)) begin ++;
    return begin;
}

std::string::const_iterator wss::skip_uri_token(std::string::const_iterator begin, std::string::const_iterator end)
{
    while(begin != end && parse::is_uri_char(*begin)) begin ++;
    return begin;  
}

std::string::const_iterator wss::skip_protocol_token(std::string::const_iterator begin, std::string::const_iterator end)
{
    while(begin != end && parse::is_protocol_char(*begin)) begin ++;
    return begin;  
}