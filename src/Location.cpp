#include "Location.hpp"

Location::Location() 
    : _path("/")
    , _root("")
    , _index("index.html")
    , _directory_listing(false)
    , _redirect("")
    , _cgi_extension("")
    , _upload_path("")
{
    // Constructor por defecto
}

Location::Location(const Location& other)
    : _path(other._path)
    , _methods(other._methods)
    , _root(other._root)
    , _index(other._index)
    , _directory_listing(other._directory_listing)
    , _redirect(other._redirect)
    , _cgi_extension(other._cgi_extension)
    , _upload_path(other._upload_path)
{
    // Constructor de copia
}

Location::~Location() {
    // Destructor
}
