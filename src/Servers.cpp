#include "../inc/Servers.hpp"

Servers::Servers(const ParsedServer &params)
    : _listen(params.listens.empty() ? Listen("0.0.0.0", 80) : params.listens[0])
    , _server_names(params.server_names)
    , _root(params.root)
    , _indexFiles(params.index_files)
    , _errorPages(params.error_pages)
    , _allow_methods(params.allow_methods)
    , _autoindex(params.autoindex)
    , _client_max_body_size(params.client_max_body_size)
    , _locations(params.locations)
{}

Servers::Servers(const ParsedServer &params, const Listen &listen)
    : _listen(Listen(listen.host, listen.port, listen.is_default))
    , _server_names(params.server_names)
    , _root(params.root)
    , _indexFiles(params.index_files)
    , _errorPages(params.error_pages)
    , _allow_methods(params.allow_methods)
    , _autoindex(params.autoindex)
    , _client_max_body_size(params.client_max_body_size)
    , _locations(params.locations)
{}

Servers::~Servers() {}

// SETTER GETTER
void Servers::setListen(const Listen &listen) { _listen = listen; }
const Listen& Servers::getListens(void) const { return _listen; }

void Servers::setServerNames(const std::vector<std::string> &serverNames) { _server_names = serverNames; }
const std::vector<std::string>& Servers::getServerNames(void) const { return _server_names; }

void Servers::setRoot(const std::string &root) { _root = root; }
const std::string& Servers::getRoot(void) const { return _root; }

void Servers::setIndexFiles(const std::vector<std::string> &indexFiles) { _indexFiles = indexFiles; }
const std::vector<std::string>& Servers::getIndexFiles(void) const { return _indexFiles; }

void Servers::setErrorPages(const std::map<int, std::string> &errorPages) { _errorPages = errorPages; }
const std::map<int, std::string>& Servers::getErrorPages(void) const { return _errorPages; }

void Servers::setAllowMethods(const std::vector<std::string> &allowMethods) { _allow_methods = allowMethods; }
const std::vector<std::string>& Servers::getAllowMethods(void) const { return _allow_methods; }

void Servers::setAutoindex(bool autoindex) { _autoindex = autoindex; }
bool Servers::getAutoindex(void) const { return _autoindex; }

void Servers::setClientMaxBodySize(const std::string &clientMaxBodySize) { _client_max_body_size = clientMaxBodySize; }
const std::string& Servers::getClientMaxBodySize(void) const { return _client_max_body_size; }

void Servers::setLocations(const std::map<std::string, Locations>& locations) { _locations = locations; }
const std::map<std::string, Locations>& Servers::getLocations(void) const { return _locations; }