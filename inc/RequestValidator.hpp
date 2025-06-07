#pragma once

#include <string>
#include "URI.hpp"
#include "HTTPRequest.hpp"
#include "ErrorContainer.hpp"

class RequestValidator 
{
public:
    RequestValidator(HTTPRequest & request, ErrorContainer & error_container); // And config

    void validate_method(HTTPMethod const & method);
    void validate_uri(URI const & uri);
    void validate_headers(FieldSection const & hdr);
    void validate_body(HTTPBody const & body);
    void validate_protocol(std::string const & protocol);
    void validate_request(HTTPRequest const & request);

private:
    ErrorContainer & _error_container;
    HTTPRequest & _request;
};