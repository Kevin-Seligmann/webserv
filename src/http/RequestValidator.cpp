#include "RequestValidator.hpp"

RequestValidator::RequestValidator(HTTPRequest & request, ErrorContainer & error_container)
:_request(request), _error_container(error_container)
{}


void RequestValidator::validate_method(HTTPMethod const & method)
{

}

void RequestValidator::validate_protocol(std::string const & protocol)
{
    //if (protocol != "HTTP/1.1")
    //    error("protocol \"" + protocol + "\"");
}

void RequestValidator::validate_uri(URI const & uri)
{
    // If host not present, is an error
}

void RequestValidator::validate_headers(FieldSection const & hdr)
{}

void RequestValidator::validate_body(HTTPBody const & body)
{}

void RequestValidator::validate_request(HTTPRequest const & request)
{}
