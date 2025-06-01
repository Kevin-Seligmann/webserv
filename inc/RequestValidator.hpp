#pragma once

#include <string>
#include "URI.hpp"
#include "HTTPMethod.hpp"
#include "HTTPHeader.hpp"
#include "HTTPRequest.hpp"
#include "HTTPBody.hpp"

class RequestValidator 
{
public:
    //RequestValidator(config)

    void validate_method(HTTPMethod const & method) const;
    void validate_uri(URI const & uri) const;
    void validate_headers(HTTPHeader const & hdr) const;
    void validate_body(HTTPBody const & body) const;
    void validate_protocol(std::string const & protocol) const;
    void validate_request(HTTPRequest const & request) const;

private:
// Config
};