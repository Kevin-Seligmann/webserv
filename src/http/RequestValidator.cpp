#include "RequestValidator.hpp"

RequestValidator::RequestValidator(HTTPRequest & request, ErrorContainer & error_container)
:_request(request), _error_container(error_container)
{}


void RequestValidator::validate_method(HTTPMethod const & method)
{
    if (method == NOMETHOD)
        _error_container.put_error("Request method", HTTPError::NOT_IMPLEMENTED);

}

void RequestValidator::validate_protocol(std::string const & protocol)
{
    //if (protocol != "HTTP/1.1")
    //    error("protocol \"" + protocol + "\"");

    // 505 version not suported
}

void RequestValidator::validate_uri(URI const & uri)
{
}

void RequestValidator::validate_headers(HTTPRequest const & request, FieldSection const & hdr)
{
    // Host header validation
    if (hdr.fields.find("host") == hdr.fields.end())
        return _error_container.put_error("The header section must contain a host header field", HTTPError::BAD_REQUEST);
    if (hdr.fields.find("host")->second.size() != 1)
        return _error_container.put_error("The host header must be a singleton", HTTPError::BAD_REQUEST);
    if (!request.uri.host.empty())
    {
        if (request.uri.host != hdr.host || request.uri.port != hdr.port)
            return _error_container.put_error("An authority component is present on URI but is different from the header's host", HTTPError::BAD_REQUEST);
    }
    // else
    // {
    //     if (!hdr.fields.find("host")->second.front().empty())
    //         return _error_container.put_error("A host is not present on the URI and the header host field is not empty", HTTPError::BAD_REQUEST);
    // }
}

void RequestValidator::validate_body(HTTPBody const & body)
{}

