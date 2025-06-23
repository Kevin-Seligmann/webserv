#include <sstream>
#include "HTTPRequest.hpp"

HTTPRequest::HTTPRequest(){}

void HTTPRequest::reset()
{
    body.reset();
    uri.reset();
    headers.reset();
    method = NOMETHOD;
    protocol = "";
}

std::string HTTPRequest::to_string()
{
    std::stringstream ss;

    print(ss);
    return ss.str();
}

void HTTPRequest::print(std::ostream & os) const
{
    os 
    << "METHOD: " << method << "\n"
    << "URI: " << uri << "\n"
    << "PROTOCOL: " << protocol << "\n"
    << "HEADERS: " << "\n" << headers
    << "BODY: " << body << "\n";
}

std::ostream & operator<<(std::ostream & os, HTTPRequest request)
{
    request.print(os);
    return os;
}

int HTTPRequest::get_port() const
{
    if (!uri.schema.empty())
    {
        if (uri.port < 0)
            return 80;
        else
            return uri.port;
    }
    if (headers.port < 0)
        return 80;
    else
        return headers.port;
}

std::string const HTTPRequest::get_host() const
{
    if (!uri.schema.empty())
        return uri.host;
    return headers.host;
}

std::string const HTTPRequest::get_path() const
{
    if (uri.path.empty())
        return "/";
    else
        return uri.path;
}
