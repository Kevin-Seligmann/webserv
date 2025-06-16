#include "RequestValidator.hpp"

RequestValidator::RequestValidator(HTTPRequest & request, ErrorContainer & error_container)
:_request(request), _error_container(error_container)
{}


void RequestValidator::validate_method(HTTPMethod const & method)
{
    if (method == NOMETHOD)
        _error_container.put_error("Request method", NOT_IMPLEMENTED);

}

void RequestValidator::validate_protocol(std::string const & protocol)
{
    if (protocol != "HTTP/1.1")
       _error_container.put_error(protocol, VERSION_NOT_SUPPORTED);
}

void RequestValidator::validate_uri(URI const & uri)
{
}

void RequestValidator::validate_headers(HTTPRequest const & request, FieldSection const & hdr)
{
    // Host header validation
    if (hdr.fields.find("host") == hdr.fields.end())
        _error_container.put_error("The header section must contain a host header field", BAD_REQUEST);
    else if (hdr.fields.find("host")->second.find(',') != std::string::npos)
        _error_container.put_error("The host header must be a singleton", BAD_REQUEST);
    else if (!request.uri.host.empty())
    {
        if (request.uri.host != hdr.host || request.uri.port != hdr.port)
            _error_container.put_error("An authority component is present on URI but is different from the header's host", BAD_REQUEST);
    }    
    // else
    // {
    //     if (!hdr.fields.find("host")->second.front().empty())
    //         return _error_container.put_error("A host is not present on the URI and the header host field is not empty", BAD_REQUEST);
    // }

    // Validate transfer encodings
    bool chunked_found = false;
    for (std::vector<CommaSeparatedFieldValue>::const_iterator it = hdr.transfer_encodings.begin(); it != hdr.transfer_encodings.end(); it ++)
    {
        if (it->name != "chunked")
             _error_container.put_error("Transfer-Encoding: " + it->name, NOT_IMPLEMENTED);
        else
        {
            chunked_found = true;
            if (it + 1 != hdr.transfer_encodings.end())
                _error_container.put_error("Transfer-Encoding: chunked must be last", BAD_REQUEST);
            if (!it->parameters.empty())
                _error_container.put_error("Transfer-Encoding: chunked can't have parameters", BAD_REQUEST);
            if (hdr.fields.find("content-length") != hdr.fields.end())
                _error_container.put_error("Transfer-Encoding and Content-Length are incompatible", BAD_REQUEST);
        }
    }
}

void RequestValidator::validate_body(HTTPBody const & body)
{}

