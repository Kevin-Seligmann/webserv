#include "RequestValidator.hpp"

ParsingError::ParsingError(std::string const & motive, std::string const & line, std::string::const_iterator const & place)
:motive(motive),line(line)
{
    this->place = (place - line.begin());
}

ParsingError::ParsingError(std::string const & motive)
:motive(motive), line(""){}

void RequestValidator::error(std::string const & what, std::string const & line, std::string::const_iterator place)
{
    _errors.push_back(ParsingError(what, line, place));
}


bool RequestValidator::is_ok() const
{
    return _errors.size() == 0;
}

void RequestValidator::reset()
{
    _errors.clear();
}


void RequestValidator::log_errors() const
{
    std::string err = "\n";
    for (std::vector<ParsingError>::const_iterator it = _errors.begin(); it != _errors.end(); it ++)
    {
        if (it->line == "")
            err += "Invalid request: Malformed " + it->motive + "\n";
        else
            err += "Invalid request: Malformed " + it->motive + ":\n"
                "\"" + it->line + "\"\n"
                " " + std::string(std::distance(it->line.begin(), it->line.begin() + it->place), ' ') 
                + Logger::RED + "^" + Logger::RESET + "\n";
    }
    Logger::getInstance().error(err);
};

void RequestValidator::error(std::string const & what)
{
    _errors.push_back(ParsingError(what));
}

void RequestValidator::validate_method(HTTPMethod const & method)
{

}

void RequestValidator::validate_protocol(std::string const & protocol)
{
    if (protocol != "HTTP/1.1")
        error("protocol \"" + protocol + "\"");
}

void RequestValidator::validate_uri(URI const & uri)
{}

void RequestValidator::validate_headers(HTTPHeader const & hdr)
{}

void RequestValidator::validate_body(HTTPBody const & body)
{}

void RequestValidator::validate_request(HTTPRequest const & request)
{}
