#pragma once

#include <iostream>
#include "StringUtil.hpp"
#include "Utils.hpp"

enum HTTPMethod 
{
    NOMETHOD, GET, POST, DELETE, PUT
};

std::ostream & operator<<(std::ostream & os, HTTPMethod method);

namespace method {
    HTTPMethod str_to_method(std::string const & str);
    std::string const method_to_str(HTTPMethod const & m);
};