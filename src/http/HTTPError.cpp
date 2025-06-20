#include "HTTPError.hpp"

HTTPError::HTTPError(std::string const & motive, Status status) :_motive(motive), _status(status){}

HTTPError::HTTPError(){_status = OK;}

Status HTTPError::status() const {return _status;}

void HTTPError::log() const {Logger::getInstance().error(status::status_to_text(_status) + ": "   + _motive);}

std::string const HTTPError::to_string() const {return status::status_to_text(_status) + ": "   + _motive;}

void HTTPError::set(std::string const & motive, Status status){_motive = motive; _status = status;}
