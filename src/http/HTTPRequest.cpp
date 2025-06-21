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
