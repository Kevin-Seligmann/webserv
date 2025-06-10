#include "ParsingUtil.hpp"
#include <cstring>

bool parse::is_digit(char c)
{return isdigit(c);}

bool parse::is_alpha(char c)
{return isalpha(c);}


bool parse::is_vchar(char c)
{return c >= 0x21 && c <= 0x7E;}

bool parse::is_whitespace(char c)
{return strchr(" \t", c);}

bool parse::is_unreserved(char c)
{return strchr("-._~", c) || is_alpha(c) || is_digit(c);}


bool parse::is_sub_delim(char c)
{return strchr("!$&'()*+,;=", c);}

bool parse::is_protocol_char(char c)
{return is_digit(c) || is_alpha(c) || c == '/' || c == '.';}

bool parse::is_pchar(char c)
{return strchr(":@%", c) || is_sub_delim(c) || is_unreserved(c);}

bool parse::is_token_char(char c)
{
    return  is_digit(c) || is_alpha(c) || strchr("!#$%&\'*+-.^_`|~", c);
}

bool parse::is_uri_char(char c)
{
    return strchr("/?#", c) || is_digit(c) || is_alpha(c) || is_pchar(c);
}

bool parse::is_token(std::string const & str)
{
    if (str.length() == 0)
        return false;
    for (std::string::const_iterator it = str.begin(); it != str.end(); it ++)
        if (!is_token_char(*it))
            return false;
    return true;
}

bool parse::is_host_char(char c)
{return strchr(".%", c) || is_sub_delim(c) || is_unreserved(c);}


bool parse::is_fragment_char(char c)
{
    return parse::is_pchar(c) || c == '/' || c == '?';
}

bool parse::is_query_char(char c)
{
    return parse::is_pchar(c) || c == '/' || c == '?';
}

bool parse::is_ascii_whitespace(char c)
{
    return strchr(" \t\v\r\f" , c);
}

bool parse::is_field_value_char(char c)
{
    return is_vchar(c) || c < 0 || c == ' ' || c == '\t';
}

void parse::first_line_sanitize(std::string & str)
{
    for (std::string::iterator it = str.begin(); it != str.end(); it ++)
        if (parse::is_ascii_whitespace(*it))
            *it = ' ';
}

void parse::sanitize_header_value(std::string::iterator start, std::string::iterator end)
{
    for (;start != end; start ++)
        if (parse::is_ascii_whitespace(*start) || (*start < 32 && *start >= 0))
            *start = ' ';
}

bool parse::is_hexa_char(char c)
{
    return (is_digit(c) || strchr("ABCDEF", std::toupper(c)));
}

char parse::hex_to_byte(char c)
{
    if (is_digit(c))
        return c - '0';
    else
        return std::toupper(c) - 'A';
}

bool parse::is_obs_text_char(char c)
{
    return  (unsigned char) c >= 0x80 && (unsigned char) c <= 0xff;
}

bool parse::is_qdtext_char(char c)
{
    return  strchr(" \t!" , c) || (c >= 0x23 && c <= 0x5b)  || (c >= 0x5d && c <= 0x7e) || is_obs_text_char(c);
}

bool parse::is_quoted_pair_char(char c)
{
    return is_vchar(c) || strchr(" \t" , c) || is_obs_text_char(c);
}
