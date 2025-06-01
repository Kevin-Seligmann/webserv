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
{return strchr("HTTP./10", c);}

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
