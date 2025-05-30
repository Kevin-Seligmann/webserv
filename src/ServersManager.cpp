#include "../inc/ServersManager.hpp"

// ServersManager
ServersManager::ServersManager() {}

ServersManager::~ServersManager() {}

// al llamar al ws.addServer... crear el HostPort
/*
bucle en ws.serversManager as server
    HostPort tmp(server.getHost(), server.getPort());
    ws.addServer(tmp, server)
*/

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

