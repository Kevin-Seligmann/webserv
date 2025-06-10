#pragma once

#include <string>
#include "Logger.hpp"



class HTTPError 
{
public:
    enum ErrorType {
        WARNING = 1, 
        BAD_REQUEST = 400,
        CONTENT_TOO_LARGE = 413,
        URI_TOO_LONG = 414,
        REQUEST_HEADER_FIELDS_TOO_LARGE = 431,
        NOT_IMPLEMENTED = 501,
        VERSION_NOT_SUPPORTED = 505,
    };

    HTTPError(std::string const & motive, std::string const & line, std::string::const_iterator const & place, ErrorType type);
    HTTPError(std::string const & motive, ErrorType type);

    void log_as_error() const;
    void log_as_warning() const;
    std::string const & get_message_from_type(ErrorType const & type) const;

private:
    std::string _motive;
    std::string _line;
    size_t _place;

    ErrorType _type;
};