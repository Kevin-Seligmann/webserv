#include "ParsingUtil.hpp"
#include <cstring>

bool parse::is_digit(char c)
{return isdigit(c);}

bool parse::is_alpha(char c)
{return isalpha(c);}


bool parse::is_vchar(char c)
{return c >= 0x21 && c <= 0x7E;}

// bool parse::is_delimiter(char c)
// {

// }

bool parse::is_token_char(char c)
{
    return 
    is_digit(c) || is_alpha(c) ||
    c == '!' || c == '#' || c == '$' || c == '%' || c == '&' ||
    c == '\'' || c == '*' || c == '+' || c == '-' || c == '.' ||
    c == '^' || c == '_' || c == '`' || c == '|' || c == '~'; 
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
