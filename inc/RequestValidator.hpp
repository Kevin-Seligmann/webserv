#pragma once

#include <string>
#include "Status.hpp"
#include "URI.hpp"
#include "HTTPRequest.hpp"
#include "HTTPError.hpp"

class RequestValidator 
{
public:
    RequestValidator(HTTPRequest & request); // And config

    void validate_method(HTTPMethod const & method);
    void validate_uri(URI const & uri);
    void validate_headers(HTTPRequest const & request, FieldSection const & hdr);
    void validate_body(HTTPBody const & body);
    void validate_protocol(std::string const & protocol);
    void validate_request(HTTPRequest const & request);

    HTTPError const * error();

private:
    HTTPError _error;
    HTTPRequest & _request;

    void put_error(std::string const & text, Status status);
};