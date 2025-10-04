#include "HTTPMethod.hpp"
#include "Logger.hpp"

std::ostream & operator<<(std::ostream & os, HTTPMethod method)
{
    switch (method)
    {
        case NOMETHOD: os << "No method"; break;
        case GET: os << "GET"; break;
        case POST: os << "POST"; break;
        case DELETE: os << "DELETE"; break;
        default: os << "Method uninitialized"; break;
    }
    return os;
}

HTTPMethod method::str_to_method(std::string const & str)
{
    if (wss::casecmp(str, "GET")) {
        return GET;
    }
    else if (wss::casecmp(str, "POST")) {
        return POST;
    }
    else if (wss::casecmp(str, "DELETE")) {
        return DELETE;
    }
    else if (wss::casecmp(str, "HEAD")) {
        return HEAD;
    }
    else {
        return NOMETHOD;
    }
}

std::string const method::method_to_str(HTTPMethod const & m)
{
    switch (m)
    {
        case GET: return "GET";
        case POST: return "POST";
        case DELETE: return "DELETE";
        case HEAD: return  "HEAD";
        default: CODE_ERR("Trying to transform an unknown method to string");
    }
}
