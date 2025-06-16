#pragma once

#include <string>
#include "Logger.hpp"
#include "Status.hpp"


class HTTPError 
{
public:
    HTTPError(std::string const & motive, std::string const & line, std::string::const_iterator const & place, Status type);
    HTTPError(std::string const & motive, Status type);

    void log_as_error() const;
    void log_as_warning() const;
    Status status() const;
    std::string const to_string() const;

private:
    std::string _motive;
    std::string _line;
    size_t _place;

    Status _type;
};