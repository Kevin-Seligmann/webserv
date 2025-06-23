#pragma once

#include <string>
#include <stdexcept>
#include <cstring>
#include "ParsingUtil.hpp"

namespace wss 
{
    void to_upper(std::string &);
    void to_lower(std::string &);
    void trim(std::string &);
    bool str_equal(std::string::iterator it, size_t n, const char *s);
    bool str_equal(std::string::iterator it, const char *s);
    bool casecmp(std::string const & s1, std::string const & s2);
    bool casecmp(std::string const & s1, size_t pos, size_t len, std::string const & s2);

    void remove_uri_segment(std::string & out);
    std::string::iterator move_uri_segment(std::string & dst, std::string & src, std::string::iterator begin);

    std::string::iterator skip_ascii_whitespace(std::string::iterator begin, std::string::iterator end);
    std::string::iterator skip_uri_token(std::string::iterator begin, std::string::iterator end);
    std::string::iterator skip_http_token(std::string::iterator begin, std::string::iterator end);
    std::string::iterator skip_protocol_token(std::string::iterator begin, std::string::iterator end);
    std::string::iterator skip_absolute_path(std::string::iterator begin, std::string::iterator end); // Any / and segments
    std::string::iterator skip_host_token(std::string::iterator begin, std::string::iterator end);
    std::string::iterator skip_port_token(std::string::iterator begin, std::string::iterator end);
    std::string::iterator skip_path_rootless(std::string::iterator begin, std::string::iterator end);
    std::string::iterator skip_query_token(std::string::iterator begin, std::string::iterator end);
    std::string::iterator skip_fragment_token(std::string::iterator begin, std::string::iterator end);
    std::string::iterator skip_schema_token(std::string::iterator begin, std::string::iterator end);
    std::string::iterator skip_ascii_whitespace_r(std::string::iterator end, std::string::iterator begin);
    std::string::iterator skip_until(std::string::iterator begin, std::string::iterator end, std::string const & str);
    std::string::iterator skip_until_dquoted_string_end(std::string::iterator begin, std::string::iterator end);
    std::string::iterator skip_whitespace(std::string::iterator begin, std::string::iterator end);
    std::string::iterator skip_hexa_token(std::string::iterator begin, std::string::iterator end);

}

