#include "HTTPError.hpp"

HTTPError::HTTPError(std::string const & motive, std::string const & line, std::string::const_iterator const & place, ErrorType type)
:_motive(motive),_line(line), _type(type)
{
    _place = (place - line.begin());
}

HTTPError::HTTPError(std::string const & motive, ErrorType type)
:_motive(motive), _line(""), _type(type){}


void HTTPError::log_as_error() const 
{
    std::string err;
    if (_line == "")
        err += get_message_from_type(_type) + ": " + _motive;
    else
        err += get_message_from_type(_type) + ": "   + _motive + ":\n"
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

std::string const & HTTPError::get_message_from_type(ErrorType const & type) const
{
    static const std::string bad_request = "Bad Request (400)";
    static const std::string content_too_large = "Content Too Large (413)";
    static const std::string uri_too_long = "URI Too Long (414)";
    static const std::string request_header_too_large = "Request Header Fields Too Large (431)";
    static const std::string not_implemented = "Not Implemented (501)";
    static const std::string default_error = "Error";

    switch (type)
    {
        case BAD_REQUEST: return bad_request;
        case CONTENT_TOO_LARGE: return content_too_large;
        case URI_TOO_LONG: return uri_too_long;
        case REQUEST_HEADER_FIELDS_TOO_LARGE: return request_header_too_large;
        case NOT_IMPLEMENTED: return not_implemented;
        default: return default_error;
    }
}