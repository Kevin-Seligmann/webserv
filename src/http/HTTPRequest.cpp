#include "HTTPRequest.hpp"

HTTPRequest::HTTPRequest()
:method(NOMETHOD){}

void HTTPRequest::reset()
{
    method = NOMETHOD;
    // body.reset();
    uri.reset();
    for (std::vector<HTTPHeader>::iterator it = headers.begin(); it != headers.end(); it ++)
        it->reset();
    protocol = "";
}

void HTTPRequest::print(std::ostream & os) const
{
    os << "Method: " << method << "\n"
    << "URI: " << uri << "\n";
    if (protocol.size() == 0)
        os << "Protocol: " << "No protocol" << "\n";
    else
        os << "Protocol: " << protocol << "\n";
    os << "Headers: ";
    if (headers.size() == 0)
        os << "No header" << "\n";
    for (std::vector<HTTPHeader>::const_iterator it = headers.begin(); it != headers.end(); it ++)
    {
        os << *it << "\n";
    }
    os << "Body: " << "\n"
    ;
}

std::ostream & operator<<(std::ostream & os, HTTPRequest request)
{
    request.print(os);
    return os;
}
