#include "../inc/Servers.hpp"

Servers::Servers(const ParsedServer &params, bool default_server)
    : _hostPort(Listen(params.host, params.port))
    , _host(params.host)
    , _port(params.port)
    , _server_name(params.server_name)
    , _root(params.root)
    , _indexFiles(params.index_files)
    , _errorPages(params.error_pages)
    , _locations(params.locations)
    , _default_server(default_server)
{
}

Servers::Servers(const ParsedServer &params, const Listen &listen)
    : _hostPort(Listen(listen.host, listen.port, listen.is_default))
    , _host(listen.host)
    , _port(listen.port)
    , _server_name(params.server_names.empty() ? "" : params.server_names[0])
    , _root(params.root)
    , _indexFiles(params.index_files)
    , _errorPages(params.error_pages)
    , _locations(params.locations)
    , _default_server(listen.is_default)
{
}

Servers::~Servers() {}

void Servers::setListen(const Listen &listen) { _hostPort.host = listen.host; _hostPort.port = listen.port; }
const Listen& Servers::getListens(void) const { return _hostPort; }
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
void Servers::setLocations(const std::map<std::string, ParsedLocations>& locations) { _locations = locations; }
const std::map<std::string, ParsedLocations>& Servers::getLocations(void) const { return _locations; }
void Servers::setDefaultServer(bool val) { _default_server = val; }
bool Servers::isDefaultServer() const { return _default_server; }