#include "../inc/ServersManager.hpp"

// HostPort
HostPort::HostPort(const std::string& host, const int port)
    : host(host)
    , port(port)
    {}

HostPort::~HostPort() {}

bool HostPort::operator==(const HostPort& other) const {
    return (host == other.host && port == other.port);
}

bool HostPort::operator<(const HostPort& other) const {
    if (host < other.host)
        return true;
    if (host > other.host)
        return false;
    return port < other.port;
}

// ServersManager
ServersManager::ServersManager() {}

ServersManager::~ServersManager() {}

void ServersManager::addServer(const HostPort& hostPort, Servers& server) {
    std::map<HostPort, std::vector<Servers> >::iterator it = serversManager.find(hostPort);
    if (it != serversManager.end()) {
        server.setDefaultServer(false);
        it->second.push_back(server);
    } else {
        server.setDefaultServer(true);
        serversManager[hostPort] = std::vector<Servers>(1, server);
    }
}

// match pendiente
/*
const Servers& ServersManager::matchRequestServer(const HttpRequest& req) {
}
*/

