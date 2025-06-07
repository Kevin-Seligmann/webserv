#include "ErrorContainer.hpp"

bool ErrorContainer::error() const {return !_errors.empty();}

void ErrorContainer::reset(){}

void ErrorContainer::put_error(std::string const & what, std::string const & line, std::string::const_iterator place)
{_errors.push_back(HTTPError(what, line, place));}

void ErrorContainer::put_error(std::string const & what)
{_errors.push_back(HTTPError(what));}

void ErrorContainer::put_warning(std::string const & what, std::string const & line, std::string::const_iterator place)
{_warnings.push_back(HTTPError(what, line, place));}

void ErrorContainer::put_warning(std::string const & what)
{_warnings.push_back(HTTPError(what));}

void ErrorContainer::log_errors() const
{
    for (std::vector<HTTPError>::const_iterator it = _errors.begin(); it != _errors.end(); it ++)
        it->log_as_error();
}

void ErrorContainer::log_warnings() const
{
    for (std::vector<HTTPError>::const_iterator it = _warnings.begin(); it != _warnings.end(); it ++)
        it->log_as_warning();
}

void ErrorContainer::log_all() const
{
    log_warnings();
    log_errors();
}