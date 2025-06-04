#include "../inc/ServersManager.hpp"

ServersManager::ServersManager() {}

ServersManager::~ServersManager() {}

void ServersManager::addServer(Servers& server) {

    Listen aux_listen = server.getListens();    
    std::map<Listen, std::vector<Servers> >::iterator it = serversManager.find(aux_listen);

    if (it != serversManager.end()) {
        aux_listen.is_default = false;
        server.setListen(aux_listen);
        it->second.push_back(server);
    } else {
        aux_listen.is_default = true;
        server.setListen(aux_listen);
        serversManager[aux_listen] = std::vector<Servers>(1, server);
    }
}

// match pendiente
/*
const Servers& ServersManager::matchRequestServer(const HttpRequest& req) {
}
*/

