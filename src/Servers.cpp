#include "../inc/Servers.hpp"

Servers::Servers(const ParsedServer &params, bool default_server)
    : _host(params.host)
    , _port(params.port)
    , _server_name(params.serverName)
    , _root(params.root)
    , _indexFiles(params.indexFiles)
    , _errorPages(params.errorPages)
    , _locations()
    , _default_server(default_server)
{
    for (size_t i = 0; i < params.locations.size(); ++i) {
        _locations.push_back(Locations(params.locations[i]));
    }
}

Servers::~Servers() {}

void Servers::setHost(const std::string &host) { _host = host; }
const std::string& Servers::getHost(void) const { return _host; }
void Servers::setPort(const int port) { _port = port; }
int Servers::getPort(void) const { return _port; }
void Servers::setServerName(const std::string &serverName) { _server_name = serverName; }
const std::string& Servers::getServerName(void) const { return _server_name; }
void Servers::setRoot(const std::string &root) { _root = root; }
const std::string& Servers::getRoot(void) const { return _root; }
void Servers::setIndexFiles(const std::vector<std::string> &indexFiles) { _indexFiles = indexFiles; }
const std::vector<std::string>& Servers::getIndexFiles(void) const { return _indexFiles; }
void Servers::setErrorPages(const std::map<int, std::string> &errorPages) { _errorPages = errorPages; }
const std::map<int, std::string>& Servers::getErrorPages(void) const { return _errorPages; }
void Servers::setLocations(const std::vector<Locations>& locations) { _locations = locations; }
const std::vector<Locations>& Servers::getLocations(void) const { return _locations; }
void Servers::setDefaultServer(bool val) { _default_server = val; }
bool Servers::isDefaultServer() const { return _default_server; }