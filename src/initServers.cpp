#include "../inc/ServersManager.hpp"
#include "../inc/Utils.hpp"
#include <iostream>
#include <sstream>

bool serversInit(ServersManager& sm, const ParsedServers& ps) {

    for (size_t i = 0; i < ps.size(); ++i) {
        const ParsedServer& config = ps[i];
        
        // Handle case where no listen directives exist - use default
        if (config.listens.empty()) {
            Listen defaultListen;
            Servers aux_server(config, defaultListen);
            sm.addServer(aux_server.getListens(), aux_server);
        } else {
            // For each listen directive, register the same server configuration
            for (size_t j = 0; j < config.listens.size(); ++j) {
                const Listen& listen = config.listens[j];
                Servers aux_server(config, listen);
                sm.addServer(aux_server.getListens(), aux_server);
            }
        }
    }
    std::cout << BLUE << "Success: servers initialized." << RESET << std::endl;
    return true;
}