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
        case PUT: os << "PUT"; break;
        default: os << "Method uninitialized"; break;
    }
    return os;
}

HTTPMethod method::str_to_method(std::string const & str)
{
    Logger::getInstance() << "str_to_method input: '" << str << "'" << std::endl;
    if (wss::casecmp(str, "GET")) {
        Logger::getInstance() << "Matched GET" << std::endl;
        return GET;
    }
    else if (wss::casecmp(str, "POST")) {
        Logger::getInstance() << "Matched POST" << std::endl;
        return POST;
    }
    else if (wss::casecmp(str, "DELETE")) {
        Logger::getInstance() << "Matched DELETE" << std::endl;
        return DELETE;
    }
    else if (wss::casecmp(str, "PUT")) {
        Logger::getInstance() << "Matched PUT" << std::endl;
        return PUT;
    }
    else if (wss::casecmp(str, "HEAD")) {
        Logger::getInstance() << "Matched HEAD" << std::endl;
        return HEAD;
    }
    else {
        Logger::getInstance() << "No match - returning NOMETHOD" << std::endl;
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
        case PUT: return  "PUT";
        case HEAD: return  "HEAD";
        default: CODE_ERR("Trying to transform an unknown method to string");
    }
}
