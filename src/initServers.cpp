#include "../inc/ServersManager.hpp"
#include "../inc/Utils.hpp"
#include <iostream>
#include <sstream>

bool serversInit(ServersManager& sm, const ParsedServers& ps) {

    for (size_t i = 0; i < ps.size(); ++i) {
        Servers aux_server(ps[i]);
        sm.addServer(aux_server.getHostPort(), aux_server); // maneja la lógica de default_server
    }
    std::cout << BLUE << "Success: servers initialized." << RESET << std::endl;
    return true;
}