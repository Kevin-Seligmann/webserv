#include "HTTPMethod.hpp"

std::ostream & operator<<(std::ostream & os, HTTPMethod method)
{
    switch (method)
    {
        case NOMETHOD: os << "No method"; break;
        case GET: os << "GET"; break;
        case POST: os << "POST"; break;
        case DELETE: os << "DELETE"; break;
        case PUT: os << "PUT"; break;
        default: os << "Method uninitialized"; break;
    }
    return os;
}

HTTPMethod method::str_to_method(std::string const & str)
{
    if (str == "GET")
        return GET;
    else if (str == "POST")
        return POST;
    else if (str == "DELETE")
        return DELETE;
    else if (str == "PUT")
        return PUT;
    else
        return NOMETHOD;
}