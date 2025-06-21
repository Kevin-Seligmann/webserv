#pragma once

#include <string>
#include "Logger.hpp"
#include "Status.hpp"


class HTTPError 
{
public:
    HTTPError();
    HTTPError(std::string const & motive, Status _status);

    void log() const;
    Status status() const;
    std::string const & msg() const;

    void set(std::string const & motive, Status _status);
    std::string const to_string() const;

private:
    std::string _motive;
    Status _status;
};