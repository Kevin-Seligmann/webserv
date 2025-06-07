#include "HTTPError.hpp"

HTTPError::HTTPError(std::string const & motive, std::string const & line, std::string::const_iterator const & place)
:_motive(motive),_line(line)
{
    _place = (place - line.begin());
}

HTTPError::HTTPError(std::string const & motive)
:_motive(motive), _line(""){}

void HTTPError::log_as_error() const 
{
    std::string err;
    if (_line == "")
        err += "Invalid request: Malformed " + _motive;
    else
        err += "Invalid request: Malformed " + _motive + ":\n"
            "\"" + _line + "\"\n"
            " " + std::string(std::distance(_line.begin(), _line.begin() + _place), ' ') 
            + Logger::RED + "^" + Logger::RESET;
    Logger::getInstance().error(err);
}

void HTTPError::log_as_warning() const 
{
    std::string err;
    if (_line == "")
        err += "Request: " + _motive;
    else
        err += "Request: " + _motive + ":\n"
            "\"" + _line + "\"\n"
            " " + std::string(std::distance(_line.begin(), _line.begin() + _place), ' ') 
            + Logger::YELLOW + "^" + Logger::RESET;
    Logger::getInstance().warning(err);
}