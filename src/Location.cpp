#include "Location.hpp"

Location::Location()
    : _path("/")
    , _match_type(UNSET)
    , _methods()
    , _root("")
    , _autoindex(AINDX_DEF_OFF)
    , _redirect("")
    , _cgi_extension("")
    , _allow_upload(false)
{
    _index.push_back("index.html");
}

Location::Location(const Location& other)
    : _path(other._path)
    , _match_type(other._match_type) 
    , _methods(other._methods)
    , _root(other._root)
    , _index(other._index)
    , _autoindex(other._autoindex)
    , _redirect(other._redirect)
    , _cgi_extension(other._cgi_extension)
    , _allow_upload(other._allow_upload)
    , _error_pages(other._error_pages)
{
}

Location::~Location() {}

bool Location::matchesPath(const std::string& path) const {
    if (_path == "/")
        return true;
    
    if (path.find(_path) == 0)
        return (path.length() == _path.length() || path[_path.length()] == '/');

    return false;
}

std::string Location::getErrorPage(int error_code) const {
    std::map<int, std::string>::const_iterator it = _error_pages.find(error_code);
    if(it != _error_pages.end()) {
        return it->second;
    }
    return "";
}