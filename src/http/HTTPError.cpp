#include "HTTPError.hpp"

HTTPError::HTTPError(std::string const & motive, std::string const & line, std::string::const_iterator const & place, Status type)
:_motive(motive),_line(line), _type(type)
{
    _place = (place - line.begin());
}

HTTPError::HTTPError(std::string const & motive, Status type)
:_motive(motive), _line(""), _type(type){}


void HTTPError::log_as_error() const 
{
    std::string err;
    if (_line == "")
        err += status::status_to_text(_type) + ": " + _motive;
    else
        err += status::status_to_text(_type) + ": "   + _motive + ":\n"
            "\"" + _line + "\"\n"
            " " + std::string(std::distance(_line.begin(), _line.begin() + _place), ' ') 
            + Logger::RED + "^" + Logger::RESET;
    Logger::getInstance().error(err);
}

void HTTPError::log_as_warning() const 
{
    std::string err;
    if (_line == "")
        err +=  _motive;
    else
        err +=  _motive + ":\n"
            "\"" + _line + "\"\n"
            " " + std::string(std::distance(_line.begin(), _line.begin() + _place), ' ') 
            + Logger::YELLOW + "^" + Logger::RESET;
    Logger::getInstance().warning(err);
}

std::string const HTTPError::to_string() const
{
    return "(DEBUG) motive: " + _motive + " line: " + _line + " status: " + status::status_to_text(_type);
}

Status HTTPError::status() const{return _type;}
