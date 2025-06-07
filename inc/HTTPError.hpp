#pragma once

#include <string>
#include "Logger.hpp"

class HTTPError 
{
public:
    HTTPError(std::string const & motive, std::string const & line, std::string::const_iterator const & place);
    HTTPError(std::string const & motive);

    void log_as_error() const;
    void log_as_warning() const;

private:
    std::string _motive;
    std::string _line;
    size_t _place;
};