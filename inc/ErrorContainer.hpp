#pragma once

#include <vector>
#include "HTTPError.hpp"

class ErrorContainer {
public:
    bool error() const;
    void reset();
    void put_error(std::string const & what, std::string const & line, std::string::const_iterator place, HTTPError::ErrorType type);
    void put_error(std::string const & what, HTTPError::ErrorType type);
    void put_warning(std::string const & what, std::string const & line, std::string::const_iterator place);
    void put_warning(std::string const & what);
    void log_errors() const;
    void log_warnings() const;
    void log_all() const;

private:
    std::vector<HTTPError> _errors;
    std::vector<HTTPError> _warnings;
    bool _error;
};