#include "RequestValidator.hpp"

void RequestValidator::put_error(std::string const & text, Status status)
{
    if (_error.status() != OK)
    {
        Logger::getInstance().warning("Found more than one error, will be logged and ignored. " + status::status_to_text(status) + " " + text + ". Actual: " + _error.to_string());
        return ;
    }
    _error.set(text, status, true);
}

HTTPError const * RequestValidator::error(){return &_error;}

RequestValidator::RequestValidator(HTTPRequest & request, HTTPError & error):_request(request), _error(error){}


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

    
void RequestValidator::validate_first_line(HTTPRequest const & request)
{
    validate_protocol(request.protocol);
    if (_error.status() == OK) {validate_uri(request.uri);}
    if (_error.status() == OK) {validate_method(request.method);}
}

void RequestValidator::validate_uri(URI const & uri)
{
    if (uri.schema != "" && uri.schema != "http")
        _error.set("Protocol not implemented: " + uri.schema, NOT_IMPLEMENTED, true);
}


void RequestValidator::validate_headers(HTTPRequest const & request, FieldSection const & hdr)
{
    (void) request;
    
    std::map<std::string, std::string>::const_iterator host = hdr.fields.find("host");

    // Host header validation
    if (host == hdr.fields.end())
        return put_error("The header section must contain a host header field", BAD_REQUEST);

    if (host->second.find(',') != std::string::npos)
        return put_error("The host header must be a single value", BAD_REQUEST);
    
    // if (!request.uri.host.empty() && (request.uri.host != hdr.host || request.uri.port != hdr.port))
    //      return put_error("The authority component on URI is different from the header's authority", BAD_REQUEST);


    // Validate transfer encodings
    for (std::vector<Coding>::const_iterator it = hdr.transfer_encodings.begin(); it != hdr.transfer_encodings.end(); it ++)
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

    // Validate Connection
    for (std::vector<std::string>::const_iterator it = hdr.connections.begin(); it != hdr.connections.end(); it ++)
        if (*it == "close")
            _request.headers.close_status = RCS_CLOSE;
        
    // Validate expectations
    for (std::vector<std::string>::const_iterator it = hdr.expectations.begin(); it != hdr.expectations.end(); it ++)
        if (*it != "100-continue")
            return put_error("Only expectation allowed is '100-continue'", EXPECTATION_FAILED);
  
    // Validate Content-Type
    if (_request.method == POST)
    {
        if (_request.headers.fields.find("content-type") == _request.headers.fields.end())
        {
            _request.headers.content_type.type = "application";
            _request.headers.content_type.subtype = "octet-stream";
        }

        std::string full_type = _request.headers.content_type.type 
                                + "/" + _request.headers.content_type.subtype;

        // early return para POST con file dentro del body, content type no matcheara
        if (full_type == "multipart/form-data" ||
            full_type == "application/x-www-form-urlencoded")
        {
            return;
        }

        // post nativo, completa validacion de content type == extension del archivo
        t_mime_conf::iterator it;
        for (it = MediaType::ACCEPTED_TYPES.begin(); it != MediaType::ACCEPTED_TYPES.end(); it ++)
        {
            if (wss::casecmp(it->first.type, _request.headers.content_type.type) &&
                wss::casecmp(it->first.subtype, _request.headers.content_type.subtype))
            {
                validate_extensions(_request.get_path(), it->second);
                break ;
            }
        }
        if (it == MediaType::ACCEPTED_TYPES.end())
            return put_error("Unsupported media type " + _request.headers.content_type.type + "/" + _request.headers.content_type.subtype, UNSUPPORTED_MEDIA_TYPE);
    }
}

bool RequestValidator::validate_extension(std::string const & filename, std::string const & extension)
{
    std::cout<< "VALIDATE: " << filename << " " << extension << std::endl;
    return wss::casecmp(filename, filename.size() - extension.size(), extension.size(), extension);
}

void RequestValidator::validate_extensions(std::string const & filename, std::vector<std::string> const & extensions)
{
    std::vector<std::string>::const_iterator it;
    for (it = extensions.begin(); it != extensions.end() && _error.status() == OK; it ++)
        if (validate_extension(filename, *it))
            break ;
    if (it == extensions.end() && !extensions.empty())
        _error.set("Mismatch between Content-Type and extension", BAD_REQUEST, true);
}
