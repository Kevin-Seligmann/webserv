#include "RequestValidator.hpp"

void RequestValidator::validate_method(HTTPMethod const & method) const
{
    if (method == NOMETHOD)
        throw std::runtime_error("Invalid method");
}

void RequestValidator::validate_protocol(std::string const & protocol) const
{
    if (protocol != "HTTP/1.1")
        throw std::runtime_error("Invalid protocol: " + protocol);
}

void RequestValidator::validate_uri(URI const & uri) const
{}

void RequestValidator::validate_headers(HTTPHeader const & hdr) const
{}

void RequestValidator::validate_body(HTTPBody const & body) const
{}

void RequestValidator::validate_request(HTTPRequest const & request) const
{}
