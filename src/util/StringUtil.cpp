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

std::string::iterator wss::skip_ascii_whitespace(std::string::iterator begin, std::string::iterator end)
{
    while(begin != end && parse::is_ascii_whitespace(*begin)) begin ++; 
    return begin;
}

std::string::iterator wss::skip_whitespace(std::string::iterator begin, std::string::iterator end)
{
    while(begin != end && (*begin == ' '|| *begin == '\t')) begin ++; 
    return begin;
}

std::string::iterator wss::skip_ascii_whitespace_r(std::string::iterator end, std::string::iterator begin)
{
    end --;
    while(begin != end && parse::is_ascii_whitespace(*end)) end --; 
    return end + 1;
}

std::string::iterator wss::skip_http_token(std::string::iterator begin, std::string::iterator end)
{
    while(begin != end && parse::is_token_char(*begin)) begin ++;
    return begin;
}

std::string::iterator wss::skip_uri_token(std::string::iterator begin, std::string::iterator end)
{
    while(begin != end && parse::is_uri_char(*begin)) begin ++;
    return begin;  
}

std::string::iterator wss::skip_protocol_token(std::string::iterator begin, std::string::iterator end)
{
    while(begin != end && parse::is_protocol_char(*begin)) begin ++;
    return begin;  
}

std::string::iterator wss::skip_path_rootless(std::string::iterator begin, std::string::iterator end)
{
    while(begin != end && parse::is_pchar(*begin)) begin ++;
    return skip_absolute_path(begin, end);
}

std::string::iterator wss::skip_absolute_path(std::string::iterator begin, std::string::iterator end)
{
    while (begin != end && *begin == '/')
    {
        begin ++;
        while(begin != end && parse::is_pchar(*begin)) begin ++;   
    }
    return begin;
}

 std::string::iterator wss::skip_host_token(std::string::iterator begin, std::string::iterator end)
{
    while (begin != end && parse::is_host_char(*begin)) begin ++;
    return begin;
}

std::string::iterator wss::skip_port_token(std::string::iterator begin, std::string::iterator end)
{
    begin ++;
    while (begin != end && parse::is_digit(*begin)) begin ++;
    return begin;
}

std::string::iterator wss::skip_query_token(std::string::iterator begin, std::string::iterator end)
{
    begin ++;
    while (begin != end && parse::is_query_char(*begin)) begin ++;
    return begin;
}

std::string::iterator wss::skip_fragment_token(std::string::iterator begin, std::string::iterator end)
{
    begin ++;
    while (begin != end && parse::is_fragment_char(*begin)) begin ++;
    return begin;
}

std::string::iterator wss::skip_schema_token(std::string::iterator begin, std::string::iterator end)
{
    while(begin != end && parse::is_alpha(*begin)) begin ++;
    return begin;
}

// S.f if sizeof(s) < n, or it + n >= str.end()
bool wss::str_equal(std::string::iterator it, size_t n, const char *s)
{
    for (int i = 0; i < n; i++)
        if (*(it + i) != s[i])
            return false;
    return true;
}

// S.f if it + n >= str.end()
bool wss::str_equal(std::string::iterator it, const char *s)
{
    for (int i = 0; s[i]; i++)
        if (*(it + i) != s[i])
            return false;
    return true;
}
 
void wss::remove_uri_segment(std::string & out)
{
    if (out.empty())
        return;

    std::string::iterator end = out.end() - 1;
    while (end != out.begin() && *end == '/')
        end --;
    while (end != out.begin() && *end != '/')
        end --;
    if (end == out.begin() && *end != '/')
        out.clear();
    else
        out = std::string(out.begin(), end);  
}

std::string::iterator wss::move_uri_segment(std::string & dst, std::string & src, std::string::iterator begin)
{
    std::string::iterator end = begin;
    if (end != src.end() && *end == '/')   
        end ++;
    while (end != src.end() && *end != '/')
        end ++;
    dst.insert(dst.end(), begin, end);
    return end;
}

std::string::iterator wss::skip_until(std::string::iterator begin, std::string::iterator end, std::string const & str)
{
    while (begin != end && str.find(*begin) == std::string::npos)
        begin ++;
    return begin;
}

std::string::iterator wss::skip_until_dquoted_string_end(std::string::iterator begin, std::string::iterator end)
{
    while (begin != end && *begin != '"')
    {
        if (*begin == '\\' && begin + 1 != end)
            begin ++;
        begin ++;
    }
    return begin;
}

std::string::iterator wss::skip_hexa_token(std::string::iterator begin, std::string::iterator end)
{
    while (begin != end && parse::is_hexa_char(*begin))
        begin ++;
    return begin;
}
