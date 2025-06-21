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
    bool is_field_value_char(char c);
    bool is_qdtext_char(char c);
    bool is_quoted_pair_char(char c);
    bool is_obs_text_char(char c);

    void first_line_sanitize(std::string::iterator begin, std::string::iterator end);
    void sanitize_header_value(std::string::iterator start, std::string::iterator end);

    char hex_to_byte(char c);

    size_t s_to_hex(std::string::iterator start, std::string::iterator end, size_t max);
}