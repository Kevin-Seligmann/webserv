#include "../inc/Locations.hpp"
#include "../inc/Parsed.hpp"

Locations::Locations(const ParsedLocations &params)
    : _path(params.path)
    , _validMethods(params.validMethods)
    , _root(params.root)
    , _autoindex(params.autoindex)
    , _cgiExtension(params.cgiExtension)
    , _cgiPath(params.cgiPath)
    , _redir(params.redir)
    , _uploadDir(params.uploadDir)
{}

Locations::~Locations() {}

void Locations::setPath(const std::string &path) { _path = path; }
const std::string& Locations::getPath(void) const { return _path; }
void Locations::setValidMethods(const std::vector<std::string> &validMethods) { _validMethods = validMethods; }
const std::vector<std::string>& Locations::getValidMethods(void) const { return _validMethods; }
void Locations::setRoot(const std::string &root) { _root = root; }
const std::string& Locations::getRoot(void) const { return _root; }
void Locations::setAutoindex(bool autoindex) { _autoindex = autoindex; }
bool Locations::getAutoindex(void) const { return _autoindex; }
void Locations::setCgiExtension(const std::string &cgiExtension) { _cgiExtension = cgiExtension; }
const std::string& Locations::getCgiExtension(void) const { return _cgiExtension; }
void Locations::setCgiPath(const std::string &cgiPath) { _cgiPath = cgiPath; }
const std::string& Locations::getCgiPath(void) const { return _cgiPath; }
void Locations::setRedir(const std::string &redir) { _redir = redir; }
const std::string& Locations::getRedir(void) const { return _redir; }
void Locations::setUploadDir(const std::string &uploadDir) { _uploadDir = uploadDir; }
const std::string& Locations::getUploadDir(void) const { return _uploadDir; }
