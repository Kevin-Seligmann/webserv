#include "RequestValidator.hpp"

void RequestValidator::put_error(std::string const & text, Status status)
{
    if (_error.status() != OK)
    {
        Logger::getInstance().warning("Found more than one error, will be logged and ignored. " + status::status_to_text(status) + " " + text);
        return ;
    }
    _error = HTTPError(text, status);
}

HTTPError const * RequestValidator::error(){return &_error;}

RequestValidator::RequestValidator(HTTPRequest & request):_request(request){}


void RequestValidator::validate_method(HTTPMethod const & method)
{
    if (method == NOMETHOD)
        return put_error("Request method not implemented", NOT_IMPLEMENTED);

}

void RequestValidator::validate_protocol(std::string const & protocol)
{
    if (protocol != "HTTP/1.1")
        return put_error("Invalid protocol: " + protocol + " only HTTP/1.1 supported", VERSION_NOT_SUPPORTED);
}

void RequestValidator::validate_uri(URI const & uri)
{
}


void RequestValidator::validate_headers(HTTPRequest const & request, FieldSection const & hdr)
{
    // Host header validation
    if (hdr.fields.find("host") == hdr.fields.end())
        return put_error("The header section must contain a host header field", BAD_REQUEST);

    if (hdr.fields.find("host")->second.find(',') != std::string::npos)
        return put_error("The host header must be a single value (Comma detected)", BAD_REQUEST);
    
    if (!request.uri.host.empty() && (request.uri.host != hdr.host || request.uri.port != hdr.port))
         return put_error("The authority component on URI has a different host than the headers", BAD_REQUEST);
    // else
    // {
    //     if (!hdr.fields.find("host")->second.front().empty())
    //         return _error_container.put_error("A host is not present on the URI and the header host field is not empty", BAD_REQUEST);
    // }

    // Validate transfer encodings
    for (std::vector<CommaSeparatedFieldValue>::const_iterator it = hdr.transfer_encodings.begin(); it != hdr.transfer_encodings.end(); it ++)
    {
        if (it->name != "chunked")
             return put_error("Not implemented Transfer-Encoding: " + it->name, NOT_IMPLEMENTED);
        else
        {
            if (it + 1 != hdr.transfer_encodings.end())
                return put_error("Transfer-Encoding: chunked must be last", BAD_REQUEST);
            if (!it->parameters.empty())
                return put_error("Transfer-Encoding: chunked can't have parameters", BAD_REQUEST);
            if (hdr.fields.find("content-length") != hdr.fields.end())
                return put_error("Transfer-Encoding and Content-Length are incompatible", BAD_REQUEST);
        }
    }
}

void RequestValidator::validate_body(HTTPBody const & body)
{}

