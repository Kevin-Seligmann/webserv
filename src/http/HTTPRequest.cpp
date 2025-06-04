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

void HTTPRequest::print(std::ostream & os) const
{
    os 
    << "\n\n$ ---------- REQUEST BEGIN: " << "\n"
    << "METHOD: " << method << "\n"
    << "URI: " << uri << "\n"
    << "PROTOCOL: " << protocol << "\n"
    << "HEADERS: " << "\n" << headers
    << "BODY: " << body << "\n$ ---------- REQUEST END\n\n"
    ;
}

std::ostream & operator<<(std::ostream & os, HTTPRequest request)
{
    request.print(os);
    return os;
}
