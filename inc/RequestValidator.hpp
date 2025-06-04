#pragma once

#include <string>
#include "URI.hpp"
#include "HTTPMethod.hpp"
#include "HTTPHeader.hpp"
#include "HTTPRequest.hpp"
#include "HTTPBody.hpp"


struct ParsingError 
{
    ParsingError(std::string const & motive, std::string const & line, std::string::const_iterator const & place);
    ParsingError(std::string const & motive);

    std::string motive;
    std::string line;
    size_t place;
};


class RequestValidator 
{
public:
    //RequestValidator(config)
    void validate_method(HTTPMethod const & method);
    void validate_uri(URI const & uri);
    void validate_headers(HTTPHeader const & hdr);
    void validate_body(HTTPBody const & body);
    void validate_protocol(std::string const & protocol);
    void validate_request(HTTPRequest const & request);
    
    bool is_ok() const;
    void log_errors() const;
    void error(std::string const & what, std::string const & line, std::string::const_iterator place);
    void error(std::string const & what);

    void reset();
private:
    std::vector<ParsingError> _errors;

// Config
};