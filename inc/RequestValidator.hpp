#pragma once

#include <string>
#include "MediaType.hpp"
#include "Status.hpp"
#include "URI.hpp"
#include "HTTPRequest.hpp"
#include "HTTPError.hpp"

class RequestValidator 
{
public:
    RequestValidator(HTTPRequest & request, HTTPError & error); 

    void validate_method(HTTPMethod const & method);
    void validate_uri(URI const & uri);
    void validate_headers(HTTPRequest const & request, FieldSection const & hdr);
    void validate_body(HTTPBody const & body);
    void validate_protocol(std::string const & protocol);
    void validate_request(HTTPRequest const & request);
    void validate_first_line(HTTPRequest const & request);
    void validate_extensions(std::string const & filename, std::vector<std::string> const & extensions);
    bool validate_extension(std::string const & filename, std::string const & extension);

    HTTPError const * error();

private:
    HTTPRequest & _request;
    HTTPError & _error;

    void put_error(std::string const & text, Status status);
};