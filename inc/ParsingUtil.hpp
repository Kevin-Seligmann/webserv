#pragma once

#include <string>

namespace parse 
{
    bool is_digit(char c);
    bool is_alpha(char c);
    bool is_vchar(char c);
    bool is_delimiter(char c);
    bool is_token_char(char c);
    bool is_token(std::string const &);
    bool is_whitespace(char c);
    bool is_uri_char(char c);
    bool is_unreserved(char c);
    bool is_sub_delim(char c);
    bool is_pchar(char c);
    bool is_protocol_char(char c);
    bool is_host_char(char c);
    bool is_fragment_char(char c);
    bool is_query_char(char c);
    bool is_ascii_whitespace(char c);
    bool is_hexa_char(char c);

    void first_line_sanitize(std::string & str);
    void sanitize_header_value(std::string::iterator start, std::string::iterator end);

    char hex_to_byte(char c);


}