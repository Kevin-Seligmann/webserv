#include "../inc/ServersManager.hpp"

// ServersManager
ServersManager::ServersManager() {}

ServersManager::~ServersManager() {}

void ServersManager::addServer(const Listen& listen, Servers& server) {

    std::map<Listen, std::vector<Servers> >::iterator it = serversManager.find(listen);

    if (it != serversManager.end()) {
        server.setDefaultServer(false);
        it->second.push_back(server);
    } 
    
    else {
        server.setDefaultServer(true);
        serversManager[listen] = std::vector<Servers>(1, server);
    }
}

// match pendiente
/*
const Servers& ServersManager::matchRequestServer(const HttpRequest& req) {
}
*/

