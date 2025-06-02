#include "../inc/ServersManager.hpp"

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

