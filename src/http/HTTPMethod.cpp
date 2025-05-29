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
