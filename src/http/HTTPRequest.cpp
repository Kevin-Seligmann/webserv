#include "HTTPRequest.hpp"

HTTPRequest::HTTPRequest(){}

void HTTPRequest::reset()
{
    body.reset();
    first_line.reset();
    headers.reset();
}

void HTTPRequest::print(std::ostream & os) const
{
    os 
    << "REQUEST: " << "\n"
    << "First line: " << first_line << "\n"
    << "Headers: " << "\n" << headers
    << "Body: " << body << "\n"
    ;
}

std::ostream & operator<<(std::ostream & os, HTTPRequest request)
{
    request.print(os);
    return os;
}
