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
}